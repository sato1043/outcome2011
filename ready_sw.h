#ifndef __ready_sw_h__
#define __ready_sw_h__

#include "rdwr.h"

typedef struct ready_sw_t_  ready_sw_t ;
typedef struct ready_sw_t_* ready_sw_tp;

struct state_t_;

struct ready_sw_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_READY_SW \
  ready_sw_t  ready_sw

#define IMPLEMENT_READY_SW(prefix) \
  , .ready_sw =                 \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_ready_sw(ready_sw_t* thr);
void destruct_ready_sw(ready_sw_t* thr);
void shutdown_ready_sw(ready_sw_t* thr);
void ready_sw(void *thr_);

#endif/*__ready_sw_h__*/
