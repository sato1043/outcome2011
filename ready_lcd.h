#ifndef __ready_lcd_h__
#define __ready_lcd_h__

#include "rdwr.h"

typedef struct ready_lcd_t_  ready_lcd_t ;
typedef struct ready_lcd_t_* ready_lcd_tp;

struct state_t_;

struct ready_lcd_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_READY_LCD \
  ready_lcd_t  ready_lcd

#define IMPLEMENT_READY_LCD(prefix) \
  , .ready_lcd =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_ready_lcd(ready_lcd_t* thr);
void destruct_ready_lcd(ready_lcd_t* thr);
void shutdown_ready_lcd(ready_lcd_t* thr);
void ready_lcd(void *thr_);

#endif/*__ready_lcd_h__*/
