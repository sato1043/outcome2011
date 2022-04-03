#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl

#include "lcddrv.h"

#include "praise_lcd.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/lcd0"

/**
 * construct_praise_lcd
 *  - the function must precede others.
 */
void construct_praise_lcd(praise_lcd_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_praise_lcd
 *  - the function must follow others.
 */
void destruct_praise_lcd(praise_lcd_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_praise_lcd
 *  - the function set terminate flag for praise_lcd thread.
 */
void shutdown_praise_lcd(praise_lcd_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * praise_lcd
 *  - thread function.
 */
void praise_lcd(void *thr_)
{
  assert(thr_);
  praise_lcd_t* thr =(praise_lcd_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  char buf[] ="@00you win!!       ";
  char *p;
  size_t sz =strlen(buf);
  size_t s;
  int cnt =-1;

  while(thr->is_running)
  {
    cnt = (cnt+1<100)? cnt+1 : 0 ;
    p = (cnt)? buf : "@00                 ";
    s = (cnt)? sz  :  19 ;
    if (write(fd,p,sz) != sz)
    {
      dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
      return;
    }
    usleep(1000);
  }
  close(fd);

  thr->pstate->rm_ctx(praise_lcd,thr_);
}
