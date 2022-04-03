#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "settings.h"

#include "repay_leds.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/ddsp0"

#define LEDPTN_UPDATE_DELAY_USEC  (100)
#define LEDPTN_UPDATE_DELAY_TIMES (10)

#define LEDPTN_MAX  (16)
static const unsigned char ledptn[LEDPTN_MAX] = 
{
    0x80 ,0xc0 ,0xe0 ,0xf0 ,0xf8 ,0xfc ,0xfe ,0xff
  , 0x7f ,0x3f ,0x1f ,0x0f ,0x07 ,0x03 ,0x01 ,0x00
};

/**
 * construct_repay_leds
 *  - the function must precede others.
 */
void construct_repay_leds(repay_leds_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_repay_leds
 *  - the function must follow others.
 */
void destruct_repay_leds(repay_leds_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_repay_leds
 *  - the function set terminate flag for repay_leds thread.
 */
void shutdown_repay_leds(repay_leds_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * repay_leds
 *  - thread function.
 */
void repay_leds(void *thr_)
{
  repay_leds_t* thr =(repay_leds_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  char buf[8+1];

  int idx =0;
  int cnt =-1;

  while(thr->is_running)
  {
    cnt =(cnt+1 < LEDPTN_UPDATE_DELAY_TIMES)? cnt+1 : 0 ;

    if (cnt==0)
    {
      memset(buf,'\0',sizeof(buf));
      atoBitPrnString(ledptn[idx],buf);

      if (write(fd,buf,8) != 8)
      {
        dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
        break;
      }

      idx =(idx+1 < LEDPTN_MAX)? idx+1 : 0 ;
    }
    usleep(LEDPTN_UPDATE_DELAY_USEC);
  }

  close(fd);

  thr->pstate->rm_ctx(repay_leds,thr_);
}
