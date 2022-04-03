#ifndef __thr_glue_h__
#define __thr_glue_h__

#include "rdwr.h"

typedef struct thr_glue_t_  thr_glue_t ;
typedef struct thr_glue_t_* thr_glue_tp;

struct state_t_;

struct thr_glue_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;
};

#define DECLERE_THR_GLUE \
  thr_glue_t  thr_glue

#define IMPLEMENT_THR_GLUE(prefix) \
  , .thr_glue =                \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
      }

void construct_thr_glue(thr_glue_t* thr);
void destruct_thr_glue(thr_glue_t* thr);
void shutdown_thr_glue(thr_glue_t* thr);
void thr_glue(void *thr_);

#endif/*__thr_glue_h__*/
