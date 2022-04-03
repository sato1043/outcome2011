#ifndef __bet_lcd_h__
#define __bet_lcd_h__

#include "rdwr.h"

typedef struct bet_lcd_t_  bet_lcd_t ;
typedef struct bet_lcd_t_* bet_lcd_tp;

struct state_t_;

struct bet_lcd_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
  long update_delay_usec;
  long update_delay_times;
};

#define DECLERE_BET_LCD \
  bet_lcd_t  bet_lcd

#define IMPLEMENT_BET_LCD(prefix,usec,times) \
  , .bet_lcd = \
      {                                           \
          .is_running =FALSE                      \
        , .pstate     =(struct state_t_*)&prefix  \
        , .update_delay_usec  =usec               \
        , .update_delay_times =times              \
      }

void construct_bet_lcd(bet_lcd_t* thr);
void destruct_bet_lcd(bet_lcd_t* thr);
void shutdown_bet_lcd(bet_lcd_t* thr);
void bet_lcd(void *thr_);

#endif/*__bet_lcd_h__*/
