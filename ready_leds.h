#ifndef __ready_leds_h__
#define __ready_leds_h__

#include "rdwr.h"

typedef struct ready_leds_t_  ready_leds_t ;
typedef struct ready_leds_t_* ready_leds_tp;

struct state_t_;

struct ready_leds_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_READY_LEDS \
  ready_leds_t  ready_leds

#define IMPLEMENT_READY_LEDS(prefix) \
  , .ready_leds =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_ready_leds(ready_leds_t* thr);
void destruct_ready_leds(ready_leds_t* thr);
void shutdown_ready_leds(ready_leds_t* thr);
void ready_leds(void *thr_);

#endif/*__ready_leds_h__*/
