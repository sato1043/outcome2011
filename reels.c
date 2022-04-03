#include "reels.h"
#include "util.h"

DECLARE_REELS_METHOD;

IMPLEMENT_REELS( 3 ,12,0,RP_DEFAULT
                   , 6,2,RP_DEFAULT
                   , 6,4,RP_DEFAULT); /* this parameters are stub. edit gamectx::alternate */

/**
 * rotation_ptn_f
 *  - typedef of a function to enumerate reel's values(aka.pattern)
 */
typedef void (*rotation_ptn_f)(int n ,int reel[NUM_REEL_PTRN]);

/**
 * enumerate_fixed
 *  - enumeration of values for already fixed reel
 */
static void enumerate_fixed(int n ,int reel[NUM_REEL_PTRN])
{
  int idx;
  for (idx =0; idx < NUM_REEL_PTRN; ++idx)
    reel[idx] =reels.reels[n].fixed_value;
}

/**
 * enumerate_static
 *  - enumeration of random values but same pattern at each time
 */
static void enumerate_static(int n ,int reel[NUM_REEL_PTRN])
{
  srandom(n);
  int idx;
  for (idx =0; idx < NUM_REEL_PTRN; ++idx)
    reel[idx] =random() % 10;
}

/**
 * enumerate_random
 *  - enumeration of random values every time
 */
static void enumerate_random(int n ,int reel[NUM_REEL_PTRN])
{
  srandom(time(NULL));
  int idx;
  for (idx =0; idx < NUM_REEL_PTRN; ++idx)
    reel[idx] =random() % 10;
}

/**
 * enumerate_increment
 *  - enumeration of values between 0 to 9 every time
 */
static void enumerate_increment(int n ,int reel[NUM_REEL_PTRN])
{
  int idx;
  for (idx =0; idx < NUM_REEL_PTRN; ++idx)
    reel[idx] =idx;
}

/**
 * enumerate_decrement
 *  - enumeration of values between 9 to 0 every time
 */
static void enumerate_decrement(int n ,int reel[NUM_REEL_PTRN])
{
  int idx;
  for (idx =0; idx < NUM_REEL_PTRN; ++idx)
    reel[idx] =(NUM_REEL_PTRN-1) -idx;
}

/**
 * ptn_gens
 *  - list of enumeration functions
 */
rotation_ptn_f ptn_gens[RP_MAX] = 
{
    enumerate_fixed
  , enumerate_static
  , enumerate_random
  , enumerate_increment
  , enumerate_decrement
};

/**
 * reels_construct
 *  - construct instance of three reels
 */
void reels_construct(
    int sps
  , int l0 ,int o0 ,rotation_ptn_t rp0
  , int l1 ,int o1 ,rotation_ptn_t rp1
  , int l2 ,int o2 ,rotation_ptn_t rp2
  )
{
  pthread_rdwr_init_np(&reels.rwlock, NULL);

  reels.slices_per_second =sps;

  reels.reels[0].limit =l0;
  reels.reels[1].limit =l1;
  reels.reels[2].limit =l2;

  reels.reels[0].offs  =o0;
  reels.reels[1].offs  =o1;
  reels.reels[2].offs  =o2;

  reels.reels[0].ptn   =rp0;
  reels.reels[1].ptn   =rp1;
  reels.reels[2].ptn   =rp2;

  int n;
  for (n =0; n < NUM_REELS; ++n)
  {
    reels.reels[n].slice =reels.reels[n].limit-1;
    reels.reels[n].curs  =NUM_REEL_PTRN-1;
    (ptn_gens[ reels.reels[n].ptn ])(n,reels.reels[n].reel);
    reels.reels[n].was_fixed   =FALSE;
    reels.reels[n].fixed_value =NOT_FIXED;
  }
}

/**
 * reels_destruct
 *  - destruct instance of three reels
 */
void reels_destruct()
{
  pthread_rdwr_destroy_np(&reels.rwlock);
}

/**
 * reels_was_fixed__
 *  - does one of reels have already stopped?
 *  - without lock
 *
 * @param n index of reel
 * @return TRUE if fixed
 */
#define reels_was_fixed__(n) (reels.reels[(n)].was_fixed)

/**
 * reels_was_fixed
 *  - does one of reels have already stopped?
 *
 * @param n index of reel
 * @return TRUE if fixed
 */
int reels_was_fixed(int n)
{
  assert(0<=n && n<NUM_REELS);

  pthread_rdwr_rlock_np(&reels.rwlock);

  int res =reels_was_fixed__(n);

  pthread_rdwr_runlock_np(&reels.rwlock);

  return res;
}

/**
 * reels_not_fixed__
 *  - doesn't one of reels have already stopped?
 *  - without lock
 *
 * @param n index of reel
 * @return TRUE if fixed
 */
#define reels_not_fixed__(n) (!reels.reels[(n)].was_fixed)

/**
 * reels_not_fixed
 *  - doesn't one of reels have already stopped?
 *
 * @param n index of reel
 * @return TRUE if fixed
 */
int reels_not_fixed(int n)
{
  assert(0<=n && n<NUM_REELS);

  pthread_rdwr_rlock_np(&reels.rwlock);

  int res =reels_not_fixed__(n);

  pthread_rdwr_runlock_np(&reels.rwlock);

  return res;
}

/**
 * reels_rotate
 *  - rotate reel
 *  - called in every slice splited by slices_per_second.
 *
 * @param str buffer to store response
 *
 * @return TRUE  rotated
 * @return FALSE not rotated
 */
int reels_rotate(char str[REEL_7SEG_STRING_LEN+1])
{
  pthread_rdwr_wlock_np(&reels.rwlock);

  int res =FALSE;
  int n;

  rotation_info_t* pi =&reels.reels[0];

  for (n =0 ; n < NUM_REELS; ++n ,++pi)
  {
    if (pi->was_fixed)
      continue;

    pi->slice =(pi->slice+1 < pi->limit)
      ? pi->slice+1 : 0 ;

    if (pi->slice == pi->offs)
    {
      pi->curs =(pi->curs+1 < NUM_REEL_PTRN)
        ? pi->curs+1 : 0 ;

      str[n] ='0'|pi->reel[ pi->curs ];

      res =TRUE;
    }
  }

  pthread_rdwr_wunlock_np(&reels.rwlock);

  return res;
}

/**
 * reels_get
 *  - convert curent reel's value to the C string.
 *
 * @param str buffer to store response
 */
int reels_get_fixed(int n)
{
  pthread_rdwr_rlock_np(&reels.rwlock);
  int res =reels.reels[n].fixed_value;
  pthread_rdwr_runlock_np(&reels.rwlock);
  return res;
}

/**
 * reels_get_string
 *  - convert curent reel's value to the C string.
 *
 * @param str buffer to store response
 */
void reels_get_string(char str[REEL_7SEG_STRING_LEN+1])
{
  pthread_rdwr_rlock_np(&reels.rwlock);
  str[0] ='0'|reels.reels[0].reel[ reels.reels[0].curs ];
  str[1] ='0'|reels.reels[1].reel[ reels.reels[1].curs ];
  str[2] ='0'|reels.reels[2].reel[ reels.reels[2].curs ];
  str[3] ='\0';
  pthread_rdwr_runlock_np(&reels.rwlock);
}

/**
 * reels_change_ptn_id
 *  - change reel's value pattern if is available.
 *  - not to update when specified reel was fixed.
 *
 * @param idx index of reel
 */
void reels_change_ptn_id(int n ,rotation_ptn_t ptn_id)
{
  assert(0<=n && n<NUM_REELS);

  pthread_rdwr_wlock_np(&reels.rwlock);

  if (!reels_was_fixed__(n))
  {
    reels.reels[n].ptn =ptn_id;
    (ptn_gens[ptn_id])(n,reels.reels[n].reel);
  }

  pthread_rdwr_wunlock_np(&reels.rwlock);
}

/**
 * reels_fix
 *  - stop and fix one of the reels
 *
 * @param n index of reel
 */
void reels_fix(int n)
{
  assert(0<=n && n<NUM_REELS);

  pthread_rdwr_wlock_np(&reels.rwlock);

  if (reels_not_fixed__(n))
  {
    reels.reels[n].was_fixed =TRUE;
    reels.reels[n].fixed_value
      =reels.reels[n].reel[ reels.reels[n].curs ];
    //reels.reels[n].ptn =RP_FIXED; //TODO:enable this line
    //reels.reels[n].curs =as is
    (ptn_gens[RP_FIXED])(n,reels.reels[n].reel);
  }

  pthread_rdwr_wunlock_np(&reels.rwlock);
}

/**
 * reels_cancel
 *  - cancel fixing one of the reels
 *
 * @param n index of reel
 */
void reels_cancel(int n)
{
  assert(0<=n && n<NUM_REELS);

  pthread_rdwr_wlock_np(&reels.rwlock);

  if (reels_was_fixed__(n))
  {
    reels.reels[n].was_fixed   =FALSE;
    reels.reels[n].fixed_value =NOT_FIXED;
    //reels.reels[n].curs =as is
    //reels.reels[n].ptn =RP_FIXED; //TODO:
    (ptn_gens[ reels.reels[n].ptn ])(n,reels.reels[n].reel);
  }

  pthread_rdwr_wunlock_np(&reels.rwlock);
}

/**
 * reels_was_reached
 *  - do two reels have same value?
 *
 * @see the reel stopping condition described in usecae 5-2-1
 */
static int reels_was_reached()
{
  pthread_rdwr_rlock_np(&reels.rwlock);
  int res =(reels.reels[0].fixed_value == reels.reels[1].fixed_value
         || reels.reels[0].fixed_value == reels.reels[2].fixed_value
         || reels.reels[1].fixed_value == reels.reels[2].fixed_value);
  pthread_rdwr_runlock_np(&reels.rwlock);
  return res;
}

/**
 * reels_was_complated__
 *  - do all fo the reels have same value?
 *  - without lock
 * @see the reel stopping condition described in usecae 5-2-1
 */
#define reels_was_complated__  \
  (reels.reels[0].fixed_value == reels.reels[1].fixed_value \
     && reels.reels[1].fixed_value == reels.reels[2].fixed_value)

/**
 * reels_was_complated
 *  - do all fo the reels have same value?
 *
 * @see the reel stopping condition described in usecae 5-2-1
 */
static int reels_was_complated()
{
  pthread_rdwr_rlock_np(&reels.rwlock);
  int res =(reels.reels[0].fixed_value == reels.reels[1].fixed_value
         && reels.reels[1].fixed_value == reels.reels[2].fixed_value);
  pthread_rdwr_runlock_np(&reels.rwlock);
  return res;
}

/**
 * reels_get_rate
 *  - tell to coin repayment rate
 *
 * @see the reel stopping condition described in usecae 5-2-4
 */
static int reels_get_rate()
{
  pthread_rdwr_rlock_np(&reels.rwlock);

  assert(reels_was_fixed__(0));
  assert(reels_was_fixed__(1));
  assert(reels_was_fixed__(2));

  int res =0;

  if (reels_was_complated__)
  {
    switch(reels.reels[0].fixed_value)
    {
    case 2:
    case 4:
    case 6:
    case 8: res =2; break;

    case 1:
    case 5:
    case 9: res =3; break;

    case 3:
    case 7: res =5; break;
    }
  }
  pthread_rdwr_runlock_np(&reels.rwlock);
  return res;
}

/**
 * reels_reset_fixed
 *  - cancel fixing all of reels
 *
 */
void reels_reset_fixed()
{
  pthread_rdwr_wlock_np(&reels.rwlock);

  int n;
  for (n =0; n < NUM_REELS; ++n)
  {
    reels.reels[n].slice =reels.reels[n].limit-1;
    reels.reels[n].curs  =NUM_REEL_PTRN-1;
    reels.reels[n].was_fixed   =FALSE;
    reels.reels[n].fixed_value =NOT_FIXED;
  }

  pthread_rdwr_wunlock_np(&reels.rwlock);
}
