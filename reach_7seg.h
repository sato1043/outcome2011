#ifndef __reach_7seg_h__
#define __reach_7seg_h__

#include "rdwr.h"
#include "reels.h"
#include <unistd.h>

typedef struct reach_7seg_t_  reach_7seg_t ;
typedef struct reach_7seg_t_* reach_7seg_tp;

struct state_t_;

struct reach_7seg_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;

  char string[REEL_7SEG_STRING_LEN+1];
};

#define DECLERE_REACH_7SEG \
  reach_7seg_t  reach_7seg

#define IMPLEMENT_REACH_7SEG(prefix) \
  , .reach_7seg = \
      {                                                     \
          .is_running =FALSE                                \
        , .pstate     =(struct state_t_*)&prefix            \
        , .string     ="   "                                \
      }

void construct_reach_7seg(struct reach_7seg_t_* thr);
void destruct_reach_7seg(struct reach_7seg_t_* thr);
void shutdown_reach_7seg(struct reach_7seg_t_* thr);
void reach_7seg(void *arg);

#endif/*__reach_7seg_h__*/
