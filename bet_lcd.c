#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl

#include "lcddrv.h"

#include "bet_lcd.h"
#include "settings.h"
#include "player.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/lcd0"

#define LCD_UPDATE_DELAY_USEC  (10)
#define LCD_UPDATE_DELAY_TIMES (10)

/**
 * construct_bet_lcd
 *  - the function must precede others.
 */
void construct_bet_lcd(bet_lcd_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_bet_lcd
 *  - the function must follow others.
 */
void destruct_bet_lcd(bet_lcd_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_bet_lcd
 *  - the function set terminate flag for bet_lcd thread.
 */
void shutdown_bet_lcd(bet_lcd_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * bet_lcd
 *  - thread function.
 */
void bet_lcd(void *thr_)
{
  assert(thr_);
  bet_lcd_t* thr =(bet_lcd_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  if (write(fd,"@01total:          ",19) != 19)
  {
    dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
    return;
  }

  long coins =0;
  char buf[9];
  size_t sz;
  int cnt =-1;

  while(thr->is_running)
  {
    cnt =(cnt+1 < thr->update_delay_times)? cnt+1 : 0 ;
    if (cnt == 0)
    {
      thr->pstate->pplayer->get_coins(&coins);

      memset(buf,'\0',sizeof(buf));
      sprintf(buf,"@61%ld                  " ,coins);
      sz =strlen(buf);

      if (write(fd,buf,sz) != sz)
      {
        dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
        break;
      }
    }
    usleep(thr->update_delay_usec);
  }
  close(fd);

  thr->pstate->rm_ctx(bet_lcd,thr_);
}
