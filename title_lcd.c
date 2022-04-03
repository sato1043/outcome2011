#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <sys/ioctl.h>//ioctl

#include "lcddrv.h"

#include "title_lcd.h"
#include "util.h"

#include "state.h"

#define DEVNAME "/dev/gpio/lcd0"

#define UPDATE_DELAY_USEC  (100)
#define UPDATE_DELAY_TIMES (10)

/**
 * construct_title_lcd
 *  - the function must precede others.
 */
void construct_title_lcd(title_lcd_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_title_lcd
 *  - the function must follow others.
 */
void destruct_title_lcd(title_lcd_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_title_lcd
 *  - the function set terminate flag for title_lcd thread.
 */
void shutdown_title_lcd(title_lcd_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * title_lcd
 *  - thread function.
 */
void title_lcd(void *thr_)
{
  assert(thr_);
  title_lcd_t* thr =(title_lcd_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  int fd;
  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"'%s':",DEVNAME);
    exit(EXIT_FAILURE);
  }

  char buf[] ="@00slotmachine No.8";
  size_t sz =strlen(buf);

  int cnt =-1;

#define BUF_PRECEDING (3)
  int i =BUF_PRECEDING -1; //i begins 3 since buf is preceding by '@nn'
  char c;

  while(thr->is_running)
  {
    cnt =(cnt+1 < UPDATE_DELAY_TIMES)? cnt+1 : 0 ;

    if (cnt==0)
    {
      i =(i+1 < sz)? i+1 : BUF_PRECEDING;

      c =buf[i];
      buf[i] ='_';

      if (write(fd,buf,sz) != sz)
      {
        dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
        return;
      }

      buf[i] =c;
    }
    usleep(UPDATE_DELAY_USEC);
  }
  close(fd);

  thr->pstate->rm_ctx(title_lcd,thr_);
#undef BUF_PRECEDING
}

#if 0
void slash(char buf[] ,size_t sz ,int fd)
{
  int i;
  char c;

  //i begins 3 since buf is preceding by '@nn'
  for(i =3; i<sz; ++i)
  {
    c =buf[i];
    buf[i] ='-';
    if (write(fd,buf,sz) != sz)
    {
      dbgprint(__FUNCTION__ ,"lcd(fd=%d) write failed" ,fd);
      return;
    }
    usleep(  50000);
    buf[i] =c;
  }
}
#endif
