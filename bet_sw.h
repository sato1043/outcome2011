#ifndef __bet_sw_h__
#define __bet_sw_h__

#include "rdwr.h"

typedef struct bet_sw_t_  bet_sw_t ;
typedef struct bet_sw_t_* bet_sw_tp;

struct state_t_;

struct bet_sw_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_BET_SW \
  bet_sw_t  bet_sw

#define IMPLEMENT_BET_SW(prefix) \
  , .bet_sw =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_bet_sw(bet_sw_t* thr);
void destruct_bet_sw(bet_sw_t* thr);
void shutdown_bet_sw(bet_sw_t* thr);
void bet_sw(void *thr_);

#endif/*__bet_sw_h__*/
