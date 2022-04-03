#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "single_sw.h"
#include "player.h"
#include "settings.h"
#include "util.h"

#include "state.h"

#define NUM_DEVNAMES (1)

/**
 * construct_single_sw
 *  - the function must precede others.
 */
void construct_single_sw(single_sw_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_single_sw
 *  - the function must follow others.
 */
void destruct_single_sw(single_sw_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_single_sw
 *  - the function set terminate flag for single_sw thread.
 */
void shutdown_single_sw(single_sw_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * single_sw
 *  - thread function.
 */
void single_sw(void *thr_)
{
#define BUFSZ (4096)
  assert(thr_);
  single_sw_t* thr =(single_sw_t*)thr_;

  assert(thr->action);

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(thr->devname,O_RDONLY|O_NONBLOCK)) == -1)
  {
    dbgprint(__FUNCTION__,"devname='%s':",thr->devname);
    exit(EXIT_FAILURE);
  }

  struct pollfd ufds[NUM_DEVNAMES];
  memset(&ufds,0,sizeof(ufds));
  ufds[0].fd =fd;
  ufds[0].events =POLLIN;

  int accepting =TRUE;

  while(thr->is_running)
  {
    if (poll(ufds, NUM_DEVNAMES, 100) == -1)
    {
      dbgprint(__FUNCTION__,"poll(ufds):");
      exit(EXIT_FAILURE);
    }

    if (ufds[0].revents & POLLIN) ;
    else
      continue;

    char buf[BUFSZ];
    if (read(ufds[0].fd,buf,BUFSZ-1) == -1)
    {
      if (errno == EAGAIN)
        continue;

      dbgprint(__FUNCTION__ ,"read failed devname='%s' fd=%d :"
          ,thr->devname ,ufds[0].fd);
      exit(EXIT_FAILURE);
    }

    dbgprint(__FUNCTION__ ,"read devname='%s' fd=%d :%s"
        ,thr->devname ,ufds[0].fd ,buf);

    if (accepting)
    {
      dbgprint(__FUNCTION__ ,"calling action - %p" ,thr->action);
      accepting =thr->action(thr);
    }

    usleep(1000);
  }

  close(fd);

  thr->pstate->rm_ctx(single_sw,thr_);
#undef BUFSZ
}
