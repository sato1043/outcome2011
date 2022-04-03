#ifndef __cointank_h__
#define __cointank_h__

#include "rdwr.h"

typedef struct cointank_t_  cointank_t;
typedef struct cointank_t_* cointank_tp;

struct gamectx_t_* pcontext;
struct settings_t_* psettings;

struct cointank_t_
{
  pthread_rdwr_t coin_rwlock;
  long coins;

  struct gamectx_t_* pcontext;
  struct settings_t_* psettings;

  void (*construct)();
  void (*destruct)();
  void (*increment)();
  void (*decrement)();
  void (*set_coins)(long coins);
  void (*get_coins)(long* pcoins);
  int (*is_capable)();
  int (*is_full)();
  int (*is_turnable)();
  void (*accept_to_bet_from_player)();
  void (*gain_from_player)();
};

extern cointank_t cointank;

#define COINTANK_CAPACITY (18)

#endif/*__cointank_h__*/
