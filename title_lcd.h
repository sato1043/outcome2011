#ifndef __title_lcd_h__
#define __title_lcd_h__

#include "rdwr.h"

typedef struct title_lcd_t_  title_lcd_t ;
typedef struct title_lcd_t_* title_lcd_tp;

struct state_t_;

struct title_lcd_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_TITLE_LCD \
  title_lcd_t  title_lcd

#define IMPLEMENT_TITLE_LCD(prefix) \
  , .title_lcd =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_title_lcd(title_lcd_t* thr);
void destruct_title_lcd(title_lcd_t* thr);
void shutdown_title_lcd(title_lcd_t* thr);
void title_lcd(void *thr_);

#endif/*__title_lcd_h__*/
