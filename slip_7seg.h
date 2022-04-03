#ifndef __slip_7seg_h__
#define __slip_7seg_h__

#include "rdwr.h"
#include "reels.h"
#include <unistd.h>

typedef struct slip_7seg_t_  slip_7seg_t ;
typedef struct slip_7seg_t_* slip_7seg_tp;

#define SLIP_7SEG_CHALENGE_MAX  (5)

struct state_t_;

struct slip_7seg_t_
{
  int is_running;
  pthread_rdwr_t rwlock;
  struct state_t_* pstate;

  char string[REEL_7SEG_STRING_LEN+1];
};

#define DECLERE_SLIP_7SEG \
  slip_7seg_t  slip_7seg

#define IMPLEMENT_SLIP_7SEG(prefix) \
  , .slip_7seg = \
      {                                                     \
          .is_running =FALSE                                \
        , .pstate     =(struct state_t_*)&prefix            \
        , .string     ="   "                                \
      }

void construct_slip_7seg(struct slip_7seg_t_* thr);
void destruct_slip_7seg(struct slip_7seg_t_* thr);
void shutdown_slip_7seg(struct slip_7seg_t_* thr);
void slip_7seg(void *arg);

#endif/*__slip_7seg_h__*/
