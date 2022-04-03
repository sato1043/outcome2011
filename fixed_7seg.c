#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "fixed_7seg.h"
#include "settings.h"
#include "util.h"

#include "state.h"

static void fixed_7seg_blinked_loop(void *arg ,int fd);

#define DEVNAME "/dev/gpio/ddsp1"

/**
 * construct_fixed_7seg
 *  - the function must precede others.
 *
 * @param thr porinter to fixed_7seg_t
 */
void construct_fixed_7seg(struct fixed_7seg_t_* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_fixed_7seg
 *  - the function must follow others.
 *
 * @param thr porinter to fixed_7seg_t
 */
void destruct_fixed_7seg(struct fixed_7seg_t_* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_fixed_7seg
 *  - the function set terminate flag for fixed_7seg thread.
 *
 * @param thr porinter to fixed_7seg_t
 */
void shutdown_fixed_7seg(struct fixed_7seg_t_* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * fixed_7seg_blinked_loop
 *  - thread function's sub loop when user specified blinking.
 */
static void fixed_7seg_blinked_loop(void *arg ,int fd)
{
  fixed_7seg_t* thr =(fixed_7seg_t*)arg;

  while(thr->is_running)
  {
    if (write(fd,"   ",FIXED_7SEG_STRING_LEN)
        != FIXED_7SEG_STRING_LEN)
    {
      dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
      exit(EXIT_FAILURE);
    }

    usleep(thr->blink_usec);

    if (write(fd,thr->string,FIXED_7SEG_STRING_LEN)
          != FIXED_7SEG_STRING_LEN)
    {
      dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
      exit(EXIT_FAILURE);
    }

    usleep(thr->blink_usec);
  }
}

/**
 * fixed_7seg
 *  - thread function.
 */
void fixed_7seg(void *arg)
{
  fixed_7seg_t* thr =(fixed_7seg_t*)arg;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  if (write(fd,thr->string,FIXED_7SEG_STRING_LEN)
        != FIXED_7SEG_STRING_LEN)
  {
    dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
    exit(EXIT_FAILURE);
  }

  if (thr->blink_usec)
  {
    fixed_7seg_blinked_loop(arg,fd);
  }
  else
  {
    while(thr->is_running)
    {
      usleep( 500000);
    }
  }

  close(fd);

  thr->pstate->rm_ctx(fixed_7seg,arg);
}
