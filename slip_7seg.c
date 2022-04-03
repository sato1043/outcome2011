#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "slip_7seg.h"
#include "settings.h"
#include "util.h"

#include "state.h"
#include "reels.h"

#define DEVNAME "/dev/gpio/ddsp1"

/**
 * construct_slip_7seg
 *  - the function must precede others.
 *
 * @param thr porinter to slip_7seg_t
 */
void construct_slip_7seg(struct slip_7seg_t_* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
  thr->pstate->preels->get_string(thr->string);
}

/**
 * destruct_slip_7seg
 *  - the function must follow others.
 *
 * @param thr porinter to slip_7seg_t
 */
void destruct_slip_7seg(struct slip_7seg_t_* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_slip_7seg
 *  - the function set terminate flag for slip_7seg thread.
 *
 * @param thr porinter to slip_7seg_t
 */
void shutdown_slip_7seg(struct slip_7seg_t_* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * slip_7seg
 *  - thread function.
 */
void slip_7seg(void *thr_)
{
  assert(thr_);
  slip_7seg_t* thr =(slip_7seg_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  int fv[NUM_REELS];
  fv[0] =thr->pstate->preels->get_fixed(0);
  fv[1] =thr->pstate->preels->get_fixed(1);
  fv[2] =thr->pstate->preels->get_fixed(2);

  int n_ra ,n_rb ,n_rx;
  if (fv[1] == fv[2])
  {
    n_rx =0;
    n_ra =1;
    n_rb =2;
  }
  else
  {
    n_ra =0;
    n_rb =(fv[0] == fv[1])
      ? 1 : 2;
    n_rx =(fv[0] == fv[1])
      ? 2 : 1;
  }

  thr->pstate->preels->get_string(thr->string);
  if (write(fd,thr->string,REEL_7SEG_STRING_LEN)
        != REEL_7SEG_STRING_LEN)
  {
    dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
    exit(EXIT_FAILURE);
  }

  thr->string[n_ra] =thr->string[n_rb] ='m';
  thr->pstate->preels->cancel(n_rx);

  int delay_usec =1000000/thr->pstate->preels->slices_per_second;

  pthread_rdwr_wlock_np(&thr->rwlock);

  int cnt =0;// < SLIP_7SEG_CHALENGE_MAX;

  while(thr->is_running)
  {
    //every delay_usec,
    //three reels are checked to display or not.
  
    if (thr->pstate->preels->rotate(thr->string))
    {
      thr->string[n_ra] =thr->string[n_rb] ='m';

      if (write(fd,thr->string,REEL_7SEG_STRING_LEN)
            != REEL_7SEG_STRING_LEN)
      {
        dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
        exit(EXIT_FAILURE);
      }

      thr->pstate->preels->fix(n_rx);
      if (thr->pstate->preels->was_complated())
        break;
      thr->pstate->preels->cancel(n_rx);

      ++cnt;
      if (SLIP_7SEG_CHALENGE_MAX <= cnt)
        break;
    }

#ifdef IN_LOCAL
    printf("string='%s' reel(0,1,2)=(%d%d%d)\n"
       ,thr->string
       ,thr->pstate->preels->reels[0].reel[ thr->pstate->preels->reels[0].curs ]
       ,thr->pstate->preels->reels[1].reel[ thr->pstate->preels->reels[1].curs ]
       ,thr->pstate->preels->reels[2].reel[ thr->pstate->preels->reels[2].curs ]
        );
#endif

    pthread_rdwr_wunlock_np(&thr->rwlock);

    usleep(delay_usec);

    pthread_rdwr_wlock_np(&thr->rwlock);
  }

  pthread_rdwr_wunlock_np(&thr->rwlock);

  close(fd);

  sleep(1);

  thr->pstate->rm_ctx(slip_7seg,thr_);
}
