#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "bet_leds.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/ddsp0"

#define LEDPTN_UPDATE_DELAY_USEC  (100)
#define LEDPTN_UPDATE_DELAY_TIMES (10)

/**
 * construct_bet_leds
 *  - the function must precede others.
 */
void construct_bet_leds(bet_leds_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_bet_leds
 *  - the function must follow others.
 */
void destruct_bet_leds(bet_leds_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_bet_leds
 *  - the function set terminate flag for bet_leds thread.
 */
void shutdown_bet_leds(bet_leds_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * bet_leds
 *  - thread function.
 */
void bet_leds(void *thr_)
{
  bet_leds_t* thr =(bet_leds_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  long stock =0;
  int m;
  char buf[9];
  unsigned char stat;

  int cnt =-1;

  while(thr->is_running)
  {
    cnt =(cnt+1 < LEDPTN_UPDATE_DELAY_TIMES)? cnt+1 : 0 ;

    if (cnt==0)
    {
      stat =0;

      thr->pstate->pcointank->get_coins(&stock);

      for (m =(stock-1)%3; 0<=m; --m)
        stat |=(1<<m);

      if (4<=stock)
        stat |= ((signed char)0x80 >> ((stock-1)/3 -1));

      memset(buf,'\0',sizeof(buf));
      atoBitPrnString(stat,buf);

      if (write(fd,buf,8) != 8)
      {
        dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
        break;
      }
    }

    usleep(LEDPTN_UPDATE_DELAY_USEC);
  }

  close(fd);

  thr->pstate->rm_ctx(bet_leds,thr_);
}
