#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl

#include "lcddrv.h"

#include "settings.h"
#include "player.h"

#include "ready_lcd.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/lcd0"

/**
 * construct_ready_lcd
 *  - the function must precede others.
 */
void construct_ready_lcd(ready_lcd_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_ready_lcd
 *  - the function must follow others.
 */
void destruct_ready_lcd(ready_lcd_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_ready_lcd
 *  - the function set terminate flag for ready_lcd thread.
 */
void shutdown_ready_lcd(ready_lcd_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * ready_lcd
 *  - thread function.
 */
void ready_lcd(void *thr_)
{
  ready_lcd_t* thr =(ready_lcd_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  while(thr->is_running)
  {
    if (write(fd,"@01Push any Button!",19) != 19)
    {
      dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
      return;
    }
    usleep( 500000);

    if (write(fd,"@01                ",19) != 19)
    {
      dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
      return;
    }
    usleep( 500000);
  }
  close(fd);

  thr->pstate->rm_ctx(ready_lcd,thr_);
}
