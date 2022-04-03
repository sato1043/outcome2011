#ifndef __praise_lcd_h__
#define __praise_lcd_h__

#include "rdwr.h"

typedef struct praise_lcd_t_  praise_lcd_t ;
typedef struct praise_lcd_t_* praise_lcd_tp;

struct state_t_;

struct praise_lcd_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_PRAISE_LCD \
  praise_lcd_t  praise_lcd

#define IMPLEMENT_PRAISE_LCD(prefix) \
  , .praise_lcd =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_praise_lcd(praise_lcd_t* thr);
void destruct_praise_lcd(praise_lcd_t* thr);
void shutdown_praise_lcd(praise_lcd_t* thr);
void praise_lcd(void *thr_);

#endif/*__praise_lcd_h__*/
