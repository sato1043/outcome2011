#ifndef __player_h__
#define __player_h__

#include "rdwr.h"

typedef struct player_t_  player_t;
typedef struct player_t_* player_tp;

struct gamectx_t_;
struct settings_t_;
struct pthread_rdwr_t_;

struct player_t_
{
  struct gamectx_t_*     pcontext;
  struct settings_t_*    psettings;

  pthread_rdwr_t coin_rwlock;

  long coins;

  void (*construct)();
  void (*destruct)();
  void (*increment)();
  void (*decrement)();
  void (*decrement__)();
  void (*set_coins)(long coins);
  void (*get_coins)(long* pcoins);
  int (*can_play)();
  int (*is_penniless)();
  void (*bet)();
};

extern player_t player;

#endif/*__player_h__*/
