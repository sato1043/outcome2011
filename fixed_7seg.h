#ifndef __fixed_7seg_h__
#define __fixed_7seg_h__

#include "rdwr.h"
#include <unistd.h>

#define FIXED_7SEG_STRING_LEN (3)

typedef struct fixed_7seg_t_  fixed_7seg_t ;
typedef struct fixed_7seg_t_* fixed_7seg_tp;

struct state_t_;

struct fixed_7seg_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;

  char string[FIXED_7SEG_STRING_LEN+1];
  useconds_t blink_usec;
};

#define DECLERE_FIXED_7SEG \
  fixed_7seg_t  fixed_7seg

#define IMPLEMENT_FIXED_7SEG(prefix,str,usec) \
  , .fixed_7seg =               \
      {                         \
          .is_running =FALSE    \
        , .pstate     =(struct state_t_*)&prefix  \
        , .string     =str      \
        , .blink_usec =usec     \
      }

void construct_fixed_7seg(struct fixed_7seg_t_* thr);
void destruct_fixed_7seg(struct fixed_7seg_t_* thr);
void shutdown_fixed_7seg(struct fixed_7seg_t_* thr);
void fixed_7seg(void *arg);

#endif/*__fixed_7seg_h__*/
