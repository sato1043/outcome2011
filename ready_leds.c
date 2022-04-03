#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl
#include <sys/poll.h>//poll

#include "settings.h"

#include "ready_leds.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/ddsp0"

#define LEDPTN_UPDATE_DELAY_USEC  (100)
#define LEDPTN_UPDATE_DELAY_TIMES (10)

static const unsigned char ledptn[][8] =
{
    {0x81,0x42,0x24,0x18,0x24,0x42,0x81,0x00}
  , {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01}
};

/**
 * construct_ready_leds
 *  - the function must precede others.
 */
void construct_ready_leds(ready_leds_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_ready_leds
 *  - the function must follow others.
 */
void destruct_ready_leds(ready_leds_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_ready_leds
 *  - the function set terminate flag for ready_leds thread.
 */
void shutdown_ready_leds(ready_leds_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * ready_leds
 *  - thread function.
 */
void ready_leds(void *thr_)
{
  ready_leds_t* thr =(ready_leds_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  char buf[9];

  unsigned char expcnt =0;

  int cnt =-1;

  while(thr->is_running)
  {
    cnt =(cnt+1 < LEDPTN_UPDATE_DELAY_TIMES)? cnt+1 : 0 ;

    if (cnt==0)
    {
      unsigned char stat =0;
      uint8_t cplexpcnt;

      if( !(expcnt&0x10) ) //上位1ビット目 off
      {
        if( !(expcnt&0x08) ) //下位4ビット目 off
        {
          stat = ledptn[0][expcnt&0x07]; //下位3ビット
          // 0000 0000 -> 0
          // 0000 0001 -> 1
          //        :
          // 0000 0110 -> 6
          // 0000 0111 -> 7
        }
        else //下位4ビット目 on
        {
          cplexpcnt = ~expcnt;
          stat = ledptn[0][cplexpcnt&0x07]; //下位3ビット反転
          // 0000 1000 -> 0000 1111 -> 7
          // 0000 1000 -> 0000 1111 -> 7
          // 0000 1001 -> 0000 1110 -> 6
          //        :
          // 0000 1110 -> 0000 1001 -> 1
          // 0000 1111 -> 0000 1000 -> 0
        }
      }
      else //上位1ビット目 on
      {
        if( !(expcnt&0x08) ) //下位4ビット目 off
        {
          stat = ledptn[1][expcnt&0x07];
          // 0001 0000 -> 0
          // 0001 0001 -> 1
          //        :
          // 0001 0110 -> 6
          // 0001 0111 -> 7
        }
        else
        {
          cplexpcnt = ~expcnt;
          stat = ledptn[1][cplexpcnt&0x07]; //下位3ビット反転
          // 0001 1000 -> 0001 1111 -> 7
          // 0001 1001 -> 0001 1110 -> 6
          //        :
          // 0001 1110 -> 0001 1001 -> 1
          // 0001 1111 -> 0001 1000 -> 0
          // その次は0010 0000 -> 0001 1000 -> 0
        }
      }
   
      memset(buf,'\0',sizeof(buf));
      atoBitPrnString(stat,buf);

      if (write(fd,buf,8) != 8)
      {
        dbgprint(__FUNCTION__ ,"ddsp0(fd=%d) write failed" ,fd);
        break;
      }

      ++expcnt;
    }
    usleep(LEDPTN_UPDATE_DELAY_USEC);
  }

  close(fd);

  thr->pstate->rm_ctx(ready_leds,thr_);
}
