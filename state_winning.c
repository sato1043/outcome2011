#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(winning);

/**
 * winning
 *  - instance of WINNING state
 */
state_winning_t winning =
{ 
  IMPLEMENT_STATE_BASE(winning)
  IMPLEMENT_PRAISE_LCD(winning)
  , .coin_increment_delay_usec =100000
  IMPLEMENT_BET_LCD(winning,10000,10)
  IMPLEMENT_REPAY_LEDS(winning)
  IMPLEMENT_FIXED_7SEG(winning,"mmm",0)
};

/**
 * winning_construct
 *  - construt instance of winning state
 */
void winning_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&winning);
  construct_praise_lcd(&winning.praise_lcd);
  construct_bet_lcd   (&winning.bet_lcd   );
  construct_repay_leds(&winning.repay_leds);
  construct_fixed_7seg(&winning.fixed_7seg);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * winning_destruct
 *  - destruct instance of winning state
 */
void winning_destruct()
{
  state_exit_wait((state_t*)&winning);

  dbgprint(__FUNCTION__ ,"BEGIN");

  destruct_praise_lcd(&winning.praise_lcd);
  destruct_bet_lcd   (&winning.bet_lcd   );
  destruct_repay_leds(&winning.repay_leds);
  destruct_fixed_7seg(&winning.fixed_7seg);

  state_destruct((state_t*)&winning);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * winning_entry
 *  - implementation for state
 */
void winning_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&winning.unitary);

  winning_construct();
  winning_add_ctx(praise_lcd,&winning.praise_lcd);
  winning_add_ctx(bet_lcd   ,&winning.bet_lcd   );
  winning_add_ctx(repay_leds,&winning.repay_leds);
  winning_add_ctx(fixed_7seg,&winning.fixed_7seg);
  pthread_mutex_unlock(&winning.unitary);

  long repay =TURNABLE_COIN_THRESHOLD
                * winning.preels->get_rate();
  long coins;
  winning.pplayer->get_coins(&coins);

  long guard =coins+repay;

  while(coins < guard)
  {
    ++coins;
    winning.pplayer->increment();
    usleep(winning.coin_increment_delay_usec);
  }

  sleep(1);

  shutdown_bet_lcd   (&winning.bet_lcd   );

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * winning_exit_state
 *  - implementation for state
 */
void winning_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&winning.unitary);
  shutdown_praise_lcd(&winning.praise_lcd);
  shutdown_bet_lcd   (&winning.bet_lcd   );
  shutdown_repay_leds(&winning.repay_leds);
  shutdown_fixed_7seg(&winning.fixed_7seg);
  winning.safely_terminated =TRUE;
  pthread_mutex_unlock(&winning.unitary);

  winning_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  winning.pcontext->alternate(WINNING,BETTING);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * winning_abort_state
 *  - implementation for state
 */
void winning_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&winning.unitary);
  shutdown_praise_lcd(&winning.praise_lcd);
  shutdown_bet_lcd   (&winning.bet_lcd   );
  shutdown_repay_leds(&winning.repay_leds);
  shutdown_fixed_7seg(&winning.fixed_7seg);
  winning.safely_terminated =TRUE;
  pthread_mutex_unlock(&winning.unitary);

  winning_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * winning_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void winning_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&winning,routine,arg);
}

/**
 * winning_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void winning_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&winning,routine,arg);

  if (arg == &winning.bet_lcd)
    winning_exit_state();
}
