#include <assert.h> //assert

#include <pthread.h>
#include "rdwr.h"
#include "tpool.h"
#include "settings.h"
#include "state.h"
#include "gamectx.h"

#include "player.h"
#include "cointank.h"

#include "util.h"

static void cointank_construct();
static void cointank_destruct();
static void cointank_increment();
static void cointank_decrement();
static void cointank_set_coins(long coins);
static void cointank_get_coins(long* pcoins);
static int cointank_is_capable();
static int cointank_is_full();
static int cointank_is_turnable();
static void cointank_accept_to_bet_from_player();
static void cointank_gain_from_player();

cointank_t cointank =
{
    .coins =0
  , .pcontext  =&gamectx
  , .psettings =&settings
  , .construct =cointank_construct
  , .destruct  =cointank_destruct
  , .increment =cointank_increment
  , .decrement =cointank_decrement
  , .set_coins =cointank_set_coins
  , .get_coins =cointank_get_coins
  , .is_capable=cointank_is_capable
  , .is_full   =cointank_is_full
  , .is_turnable =cointank_is_turnable
  , .accept_to_bet_from_player=cointank_accept_to_bet_from_player
  , .gain_from_player =cointank_gain_from_player
};

/**
 * cointank_construct
 *  - construct instance of cointank
 */
void cointank_construct()
{
  pthread_rdwr_init_np(&cointank.coin_rwlock, NULL);
  cointank_set_coins(0);
}

/**
 * cointank_destruct
 *  - destruct instance of cointank
 */
void cointank_destruct()
{
  pthread_rdwr_destroy_np(&cointank.coin_rwlock);
}

/**
 * cointank_increment__
 *  - make cointank increment his stock
 *  - without lock
 */
#define cointank_increment__ (++cointank.coins)

/**
 * cointank_increment
 *  - make cointank increment his stock
 */
void cointank_increment()
{
  pthread_rdwr_wlock_np(&cointank.coin_rwlock);

  cointank_increment__;

  pthread_rdwr_wunlock_np(&cointank.coin_rwlock);
}

/**
 * cointank_decrement__
 *  - make cointank decrement his stock
 *  - without lock
 */
#define cointank_decrement__ (--cointank.coins)

/**
 * cointank_decrement
 *  - make cointank decrement his stock
 */
void cointank_decrement()
{
  pthread_rdwr_wlock_np(&cointank.coin_rwlock);

  cointank_decrement__;

  pthread_rdwr_wunlock_np(&cointank.coin_rwlock);
}

/**
 * cointank_set_coins
 *  - make cointank assign a paticular quantity.
 *
 * @param coins to assign
 */
void cointank_set_coins(long coins)
{
  pthread_rdwr_wlock_np(&cointank.coin_rwlock);

  cointank.coins =coins;

  pthread_rdwr_wunlock_np(&cointank.coin_rwlock);
}

/**
 * cointank_get_coins
 *  - make cointank to retreive his stock
 *
 * @param pcoins pointer to retrieve a value
 */
void cointank_get_coins(long* pcoins)
{
  pthread_rdwr_rlock_np(&cointank.coin_rwlock);

  *pcoins =cointank.coins;

  pthread_rdwr_runlock_np(&cointank.coin_rwlock);
}

/**
 * cointank_is_capable__
 *  - can cointank get an additional one?
 *  - without lock
 */
#define cointank_is_capable__  (cointank.coins < COINTANK_CAPACITY)

/**
 * cointank_is_capable
 *  - can cointank get an additional one?
 */
int cointank_is_capable()
{
  pthread_rdwr_rlock_np(&cointank.coin_rwlock);

  int res = cointank_is_capable__;

  pthread_rdwr_runlock_np(&cointank.coin_rwlock);

  return res;
}

/**
 * cointank_is_full__
 *  - do cointank have no space?
 *  - without lock
 */
#define cointank_is_full__  (COINTANK_CAPACITY <= cointank.coins)

/**
 * cointank_is_full
 *  - do cointank have no space?
 */
int cointank_is_full()
{
  pthread_rdwr_rlock_np(&cointank.coin_rwlock);

  int res = cointank_is_full__;

  pthread_rdwr_runlock_np(&cointank.coin_rwlock);

  return res;
}

/**
 * cointank_is_turnable__
 *  - can slot rotation?
 *  - without lock
 */
#define cointank_is_turnable__  (TURNABLE_COIN_THRESHOLD <= cointank.coins && cointank.coins <= COINTANK_CAPACITY)

/**
 * cointank_is_turnable
 *  - can slot rotation?
 */
int cointank_is_turnable()
{
  pthread_rdwr_rlock_np(&cointank.coin_rwlock);

  int res = cointank_is_turnable__;

  pthread_rdwr_runlock_np(&cointank.coin_rwlock);

  return res;
}

/**
 * cointank_accept_to_bet_from_player
 *  - make cointank to accept to bet from player
 *  - player makes cointank to accept.
 */
static void cointank_accept_to_bet_from_player()
{
  pthread_rdwr_wlock_np(&cointank.coin_rwlock);

  if (cointank_is_capable__)
  {
    //so player can bet since cointank is able to suck from player.

    player.decrement__();
    cointank_increment__;

    if (cointank_is_turnable__)
      betting.begin_wait();
  }

  pthread_rdwr_wunlock_np(&cointank.coin_rwlock);
}

/**
 * gain_from_player
 *  - make cointank to gain from player
 *  - player throw coins into cointank, then cointank suck it.
 */
static void cointank_gain_from_player()
{
  pthread_rdwr_wlock_np(&cointank.coin_rwlock);

  cointank_decrement__;
  cointank_decrement__;
  cointank_decrement__;

  pthread_rdwr_wunlock_np(&cointank.coin_rwlock);
}
