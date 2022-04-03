#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(holding);

/**
 * holding
 *  - instance of HOLDING state
 */
state_holding_t holding =
{ 
  IMPLEMENT_STATE_BASE(holding)
  IMPLEMENT_TITLE_LCD(holding)
  IMPLEMENT_BET_LCD(holding,1000,100)
  IMPLEMENT_READY_LEDS(holding)
  IMPLEMENT_SLIP_7SEG(holding)
};

/**
 * holding_construct
 *  - construt instance of holding state
 */
void holding_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&holding);
  construct_title_lcd (&holding.title_lcd );
  construct_bet_lcd   (&holding.bet_lcd   );
  construct_ready_leds(&holding.ready_leds);
  construct_slip_7seg (&holding.slip_7seg );

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * holding_destruct
 *  - destruct instance of holding state
 */
void holding_destruct()
{
  state_exit_wait((state_t*)&holding);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_title_lcd (&holding.title_lcd );
  destruct_bet_lcd   (&holding.bet_lcd   );
  destruct_ready_leds(&holding.ready_leds);
  destruct_slip_7seg (&holding.slip_7seg );

  state_destruct((state_t*)&holding);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * holding_entry
 *  - implementation for state
 */
void holding_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&holding.unitary);
  holding_construct();
  holding.add_ctx(title_lcd ,&holding.title_lcd );
  holding.add_ctx(bet_lcd   ,&holding.bet_lcd   );
  holding.add_ctx(ready_leds,&holding.ready_leds);
  holding.add_ctx(slip_7seg ,&holding.slip_7seg );
  pthread_mutex_unlock(&holding.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * holding_exit_state
 *  - implementation for state
 */
void holding_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&holding.unitary);
  shutdown_title_lcd (&holding.title_lcd );
  shutdown_bet_lcd   (&holding.bet_lcd   );
  shutdown_ready_leds(&holding.ready_leds);
  shutdown_slip_7seg (&holding.slip_7seg );
  holding.safely_terminated =TRUE;
  pthread_mutex_unlock(&holding.unitary);

  holding_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  if (holding.preels->was_complated())
    holding.pcontext->alternate(HOLDING,WINNING);
  else
  {
    if (holding.pplayer->can_play())
      holding.pcontext->alternate(HOLDING,BETTING);
    else
      holding.pcontext->alternate(HOLDING,PREPARING);
  }

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * holding_abort_state
 *  - implementation for state
 */
void holding_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&holding.unitary);
  shutdown_title_lcd (&holding.title_lcd );
  shutdown_bet_lcd   (&holding.bet_lcd   );
  shutdown_ready_leds(&holding.ready_leds);
  shutdown_slip_7seg (&holding.slip_7seg );
  holding.safely_terminated =TRUE;
  pthread_mutex_unlock(&holding.unitary);

  holding_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * holding_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void holding_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&holding,routine,arg);
}

/**
 * holding_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void holding_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&holding,routine,arg);

  if (arg == &holding.slip_7seg)
    holding_exit_state();
}
