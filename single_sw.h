#ifndef __single_sw_h__
#define __single_sw_h__

#include "rdwr.h"
#include <linux/limits.h>

typedef struct single_sw_t_  single_sw_t ;
typedef struct single_sw_t_* single_sw_tp;

struct state_t_;

struct single_sw_t_
{
  int              is_running;
  pthread_rdwr_t   rwlock;
  struct state_t_* pstate;
  char             devname[PATH_MAX];
  int  (*action)(single_sw_t* thr); //return 0 if you don't want to repeat
  int              index;
};

#define DECLERE_SINGLE_SW(quantity) \
  single_sw_t  single_sw[quantity]

#define IMPLEMENT_SINGLE_SW(idx,prefix,path,func) \
  , .single_sw[idx] =           \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
        , .devname    =path     \
        , .action     =func     \
        , .index      =idx      \
      }

void construct_single_sw(single_sw_t* thr);
void destruct_single_sw(single_sw_t* thr);
void shutdown_single_sw(single_sw_t* thr);
void single_sw(void *thr_);

#endif/*__single_sw_h__*/
