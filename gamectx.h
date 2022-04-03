#ifndef __gamectx_h__
#define __gamectx_h__

#include "rdwr.h"
#include "tpool.h"

typedef struct gamectx_t_  gamectx_t ;
typedef struct gamectx_t_* gamectx_tp;

struct state_t_;
enum state_type_t_;
struct settings_t_;

struct gamectx_t_
{
  tpool_t tpool;
  pthread_rdwr_t rwlock;
  struct state_t_*        pcurrent_state;

  void (*construct)(struct settings_t_* psettings);
  void (*destruct)();
  void (*add_ctx)(void (*routine)(void *) ,void *arg);
  void (*alternate)(enum state_type_t_ from, enum state_type_t_ to);
};

extern gamectx_t gamectx;

#endif/*__gamectx_h__*/
