#ifndef __start_sw_h__
#define __start_sw_h__

#include "rdwr.h"

typedef struct start_sw_t_  start_sw_t ;
typedef struct start_sw_t_* start_sw_tp;

struct state_t_;

struct start_sw_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_START_SW \
  start_sw_t  start_sw

#define IMPLEMENT_START_SW(prefix) \
  , .start_sw =                 \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_start_sw(start_sw_t* thr);
void destruct_start_sw(start_sw_t* thr);
void shutdown_start_sw(start_sw_t* thr);
void start_sw(void *thr_);

#endif/*__start_sw_h__*/
