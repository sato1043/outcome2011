#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "settings.h"
#include "util.h"

#include "state.h"
#include "reels.h"

#include "roll_7seg.h"

#define DEVNAME "/dev/gpio/ddsp1"

/**
 * construct_roll_7seg
 *  - the function must precede others.
 *
 * @param thr porinter to roll_7seg_t
 */
void construct_roll_7seg(struct roll_7seg_t_* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
  thr->pstate->preels->get_string(thr->string);
}

/**
 * destruct_roll_7seg
 *  - the function must follow others.
 *
 * @param thr porinter to roll_7seg_t
 */
void destruct_roll_7seg(struct roll_7seg_t_* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_roll_7seg
 *  - the function set terminate flag for roll_7seg thread.
 *
 * @param thr porinter to roll_7seg_t
 */
void shutdown_roll_7seg(struct roll_7seg_t_* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * roll_7seg
 *  - thread function.
 */
void roll_7seg(void *thr_)
{
  assert(thr_);
  roll_7seg_t* thr =(roll_7seg_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  thr->pstate->preels->get_string(thr->string);
  if (write(fd,thr->string,REEL_7SEG_STRING_LEN)
        != REEL_7SEG_STRING_LEN)
  {
    dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
    exit(EXIT_FAILURE);
  }

  int delay_usec =1000000/thr->pstate->preels->slices_per_second;

  pthread_rdwr_wlock_np(&thr->rwlock);

  while(thr->is_running)
  {
    //every delay_usec,
    //three reels are checked to display or not.
  
    if (thr->pstate->preels->rotate(thr->string))
    {
      if (write(fd,thr->string,REEL_7SEG_STRING_LEN)
            != REEL_7SEG_STRING_LEN)
      {
        dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
        exit(EXIT_FAILURE);
      }

#ifdef IN_LOCAL
    printf("string='%s' reel(0,1,2)=(%d%d%d)\n"
       ,thr->string
       ,thr->pstate->preels->reels[0].reel[ thr->pstate->preels->reels[0].curs ]
       ,thr->pstate->preels->reels[1].reel[ thr->pstate->preels->reels[1].curs ]
       ,thr->pstate->preels->reels[2].reel[ thr->pstate->preels->reels[2].curs ]
        );
#endif
    }

    pthread_rdwr_wunlock_np(&thr->rwlock);

    usleep(delay_usec);

    pthread_rdwr_wlock_np(&thr->rwlock);
  }

  pthread_rdwr_wunlock_np(&thr->rwlock);

  close(fd);

  thr->pstate->rm_ctx(roll_7seg,thr_);
}
