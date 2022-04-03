#ifndef __repay_leds_h__
#define __repay_leds_h__

#include "rdwr.h"

typedef struct repay_leds_t_  repay_leds_t ;
typedef struct repay_leds_t_* repay_leds_tp;

struct state_t_;

struct repay_leds_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_REPAY_LEDS \
  repay_leds_t  repay_leds

#define IMPLEMENT_REPAY_LEDS(prefix) \
  , .repay_leds =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_repay_leds(repay_leds_t* thr);
void destruct_repay_leds(repay_leds_t* thr);
void shutdown_repay_leds(repay_leds_t* thr);
void repay_leds(void *thr_);

#endif/*__repay_leds_h__*/
