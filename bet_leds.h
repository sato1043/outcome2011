#ifndef __bet_leds_h__
#define __bet_leds_h__

#include "rdwr.h"

typedef struct bet_leds_t_  bet_leds_t ;
typedef struct bet_leds_t_* bet_leds_tp;

struct state_t_;

struct bet_leds_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_BET_LEDS \
  bet_leds_t  bet_leds

#define IMPLEMENT_BET_LEDS(prefix) \
  , .bet_leds =                 \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_bet_leds(bet_leds_t* thr);
void destruct_bet_leds(bet_leds_t* thr);
void shutdown_bet_leds(bet_leds_t* thr);
void bet_leds(void *thr_);

#endif/*__bet_leds_h__*/
