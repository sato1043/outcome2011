#include "state.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

static void player_construct();
static void player_destruct();
static void player_increment();
static void player_decrement();
static void player_decrement__();
static void player_set_coins(long coins);
static void player_get_coins(long* pcoins);
static int player_can_play();
static int player_is_penniless();
static void player_bet();

/**
 * plaer
 *  - global instance of player
 */
player_t player =
{
    .coins =0
  , .pcontext  =&gamectx
  , .psettings =&settings
  , .construct =player_construct
  , .destruct  =player_destruct
  , .increment =player_increment
  , .decrement =player_decrement
  , .decrement__ =player_decrement__
  , .set_coins =player_set_coins
  , .get_coins =player_get_coins
  , .can_play  =player_can_play
  , .is_penniless =player_is_penniless
  , .bet       =player_bet
};

/**
 * player_construct
 *  - construct instance of player
 */
void player_construct()
{
  pthread_rdwr_init_np(&player.coin_rwlock, NULL);
  player_set_coins(player.psettings->coins);
}

/**
 * player_destruct
 *  - destruct instance of player
 */
void player_destruct()
{
  pthread_rdwr_destroy_np(&player.coin_rwlock);
}

/**
 * player_increment
 *  - increment player's coin(s)
 */
void player_increment()
{
  pthread_rdwr_wlock_np(&player.coin_rwlock);

  ++player.coins;

  pthread_rdwr_wunlock_np(&player.coin_rwlock);
}

/**
 * player_decrement__
 *  - decrement player's coin(s)
 *  - without lock
 */
void player_decrement__()
{
  --player.coins;
}

/**
 * player_decrement
 *  - decrement player's coin(s)
 */
void player_decrement()
{
  pthread_rdwr_wlock_np(&player.coin_rwlock);

  --player.coins;

  pthread_rdwr_wunlock_np(&player.coin_rwlock);
}

/**
 * player_set_coins
 *  - assignment player's coin(s)
 *
 * @param coins to assign
 */
void player_set_coins(long coins)
{
  pthread_rdwr_wlock_np(&player.coin_rwlock);

  player.coins =coins;

  pthread_rdwr_wunlock_np(&player.coin_rwlock);
}

/**
 * player_get_coins
 *  - get amount of player's coin(s)
 *
 * @param pcoins pointer to get into amount of coins
 */
void player_get_coins(long* pcoins)
{
  pthread_rdwr_rlock_np(&player.coin_rwlock);

  *pcoins =player.coins;

  pthread_rdwr_runlock_np(&player.coin_rwlock);
}

/**
 * player_can_play__
 *  - can player continue to play?
 *  - without lock
 *
 * @return TRUE if player can play
 * @return FLASE if player can not play
 */
#define player_can_play__ (3 <= (player.coins + cointank.coins))

/**
 * player_can_play
 *  - can player continue to play?
 *
 * @return TRUE if player can play
 * @return FLASE if player can not play
 */
int player_can_play()
{
  pthread_rdwr_rlock_np(&player.coin_rwlock);

  //int res = player_can_play__;

  long stock;
  cointank.get_coins(&stock);

  int res = (3 <= (player.coins + stock));
  dbgprint(__FUNCTION__,"player.coins=%ld cointank.coins=%ld res=%d"
      ,player.coins ,stock ,res);

  pthread_rdwr_runlock_np(&player.coin_rwlock);

  return res;
}

/**
 * player_is_penniless__
 *  - is player penniless?
 *  - without lock
 *
 * @return TRUE if player has no coin.
 * @return FLASE if player has any coin.
 */
#define player_is_penniless__ (player.coins == 0)

/**
 * player_is_penniless
 *  - is player penniless?
 *
 * @return TRUE if player has no coin.
 * @return FLASE if player has any coin.
 */
int player_is_penniless()
{
  pthread_rdwr_rlock_np(&player.coin_rwlock);

  int res = player_is_penniless__;

  pthread_rdwr_runlock_np(&player.coin_rwlock);

  return res;
}

/**
 * player_bet
 *  - make player bet
 */
void player_bet()
{
  pthread_rdwr_wlock_np(&player.coin_rwlock);
    // i know it's dirty trick, 
    // but i don't know how the recursive lock and cond-var work well simultaneusly.
    // our readers-writer lock use these.

  if (!player_is_penniless__) //TODO:recursive lock
  {
    cointank.accept_to_bet_from_player();
  }

  pthread_rdwr_wunlock_np(&player.coin_rwlock);
}
