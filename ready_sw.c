#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "ready_sw.h"
#include "settings.h"
#include "util.h"

#include "state.h"

#define NUM_DEVNAMES  (5)
char devnames[NUM_DEVNAMES][32] =
{
    "/dev/gpio/swc0"
  , "/dev/gpio/swc1"
  , "/dev/gpio/swc2"
  , "/dev/gpio/swc3"
  , "/dev/gpio/swc4"
};

/**
 * construct_ready_sw
 *  - the function must precede others.
 */
void construct_ready_sw(ready_sw_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_ready_sw
 *  - the function must follow others.
 */
void destruct_ready_sw(ready_sw_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_ready_sw
 *  - the function set terminate flag for ready_sw thread.
 */
void shutdown_ready_sw(ready_sw_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * ready_sw
 *  - thread function.
 */
void ready_sw(void *thr_)
{
#define BUFSZ (4096)
  assert(thr_);
  ready_sw_t* thr =(ready_sw_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int idx;
  int fds[NUM_DEVNAMES];
  for (idx =0; idx < NUM_DEVNAMES; ++idx)
  {
    if ((fds[idx] =open(devnames[idx] ,O_RDONLY|O_NONBLOCK)) == -1)
    {
      dbgprint(__FUNCTION__,"devnames[%d]='%s':",idx,devnames[idx]);
      exit(EXIT_FAILURE);
    }
  }

  struct pollfd ufds[NUM_DEVNAMES];
  memset(&ufds,0,sizeof(ufds));
  for (idx =0; idx < NUM_DEVNAMES; ++idx)
  {
    ufds[idx].fd =fds[idx];
    ufds[idx].events =POLLIN;
  }

  int is_acceptable =TRUE;

  while(thr->is_running)
  {
    if (poll(ufds, NUM_DEVNAMES, 100) == -1)
    {
      dbgprint(__FUNCTION__,"poll(ufds):");
      exit(EXIT_FAILURE);
    }

    for (idx =0; idx < NUM_DEVNAMES; ++idx)
    {
      if (ufds[idx].revents & POLLIN) ;
      else
        continue;

      char buf[BUFSZ];
      if (read(ufds[idx].fd,buf,BUFSZ-1) == -1)
      {
        if (errno == EAGAIN)
          continue;

        dbgprint(__FUNCTION__ ,"read failed devnames[%d]='%s' fd=%d :"
            ,idx ,devnames[idx] ,ufds[idx].fd);
        exit(EXIT_FAILURE);
      }

      dbgprint(__FUNCTION__ ,"read devnames[%d]='%s' fd=%d :%s"
          ,idx ,devnames[idx] ,ufds[idx].fd ,buf);

      if (is_acceptable)
      {
        is_acceptable =FALSE;
        shutdown_ready_sw(thr);
      }
    }

    usleep(1000);
  }

  for (idx =0; idx < NUM_DEVNAMES; ++idx)
    close(fds[idx]);

  thr->pstate->rm_ctx(ready_sw,thr_);

  if (!is_acceptable)
    thr->pstate->exit_state();

#undef BUFSZ
}
