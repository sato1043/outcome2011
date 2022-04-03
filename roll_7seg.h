#ifndef __roll_7seg_h__
#define __roll_7seg_h__

#include "rdwr.h"
#include "reels.h"
#include <unistd.h>

typedef struct roll_7seg_t_  roll_7seg_t ;
typedef struct roll_7seg_t_* roll_7seg_tp;

struct state_t_;

struct roll_7seg_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;

  char string[REEL_7SEG_STRING_LEN+1];
};

#define DECLERE_ROLL_7SEG \
  roll_7seg_t  roll_7seg

#define IMPLEMENT_ROLL_7SEG(prefix) \
  , .roll_7seg = \
      {                                                     \
          .is_running =FALSE                                \
        , .pstate     =(struct state_t_*)&prefix            \
        , .string     ="   "                                \
      }

void construct_roll_7seg(struct roll_7seg_t_* thr);
void destruct_roll_7seg(struct roll_7seg_t_* thr);
void shutdown_roll_7seg(struct roll_7seg_t_* thr);
void roll_7seg(void *arg);

#endif/*__roll_7seg_h__*/
