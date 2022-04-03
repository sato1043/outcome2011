#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "bet_sw.h"
#include "player.h"
#include "settings.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/swc4"
#define NUM_DEVNAMES (1)

/**
 * construct_bet_sw
 *  - the function must precede others.
 */
void construct_bet_sw(bet_sw_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_bet_sw
 *  - the function must follow others.
 */
void destruct_bet_sw(bet_sw_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_bet_sw
 *  - the function set terminate flag for bet_sw thread.
 */
void shutdown_bet_sw(bet_sw_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * bet_sw
 *  - thread function.
 */
void bet_sw(void *thr_)
{
#define BUFSZ (4096)
  assert(thr_);
  bet_sw_t* thr =(bet_sw_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_RDONLY|O_NONBLOCK)) == -1)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  struct pollfd ufds[NUM_DEVNAMES];
  memset(&ufds,0,sizeof(ufds));
  ufds[0].fd =fd;
  ufds[0].events =POLLIN;

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

      dbgprint(__FUNCTION__ ,"read failed devname='%s' fd=%d:"
          ,DEVNAME ,ufds[0].fd);
      exit(EXIT_FAILURE);
    }

    dbgprint(__FUNCTION__ ,"read devname='%s' fd=%d :%s"
        ,DEVNAME ,ufds[0].fd ,buf);

    thr->pstate->pplayer->bet();

    usleep(1000);
  }

  close(fd);

  thr->pstate->rm_ctx(bet_sw,thr_);
#undef BUFSZ
}
