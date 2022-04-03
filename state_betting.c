#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(betting);

static void betting_begin_wait();

/**
 * bet_action
 *  - handler for bet swich down
 *
 * @param thr thread context
 */
static int bet_action(single_sw_t* thr)
{
  thr->pstate->pplayer->bet();
  return TRUE;
    //return 0 if you don't want to repeat
}

/**
 * start_action
 *  - handler for start swich down
 *
 * @param thr thread context
 */
static int start_action(single_sw_t* thr)
{
  pthread_mutex_lock(&betting.unitary);
  shutdown_single_sw(thr);
  betting.is_startable =TRUE;
  pthread_mutex_unlock(&betting.unitary);
  return FALSE;
    //return 0 if you don't want to repeat
}

/**
 * betting
 *  - instance of BETTING state
 */
state_betting_t betting   =
{ 
  IMPLEMENT_STATE_BASE(betting)
  IMPLEMENT_TITLE_LCD(betting)
  IMPLEMENT_BET_LCD(betting,1000,100)
  IMPLEMENT_BET_LEDS(betting)
  IMPLEMENT_FIXED_7SEG(betting,"nnn",0)
  , .is_turnable =FALSE
  , .is_startable =FALSE
  , .begin_wait =betting_begin_wait
  IMPLEMENT_SINGLE_SW(0,betting,"/dev/gpio/swc4",bet_action)
  IMPLEMENT_SINGLE_SW(1,betting,"/dev/gpio/swc3",start_action)
};

/**
 * betting_construct
 *  - construt instance of betting state
 */
void betting_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&betting);
  construct_title_lcd (&betting.title_lcd );
  construct_bet_lcd   (&betting.bet_lcd   );
  construct_bet_leds  (&betting.bet_leds  );
  construct_fixed_7seg(&betting.fixed_7seg);
  construct_single_sw (&betting.single_sw[0]);
  construct_single_sw (&betting.single_sw[1]);
  betting.is_turnable =FALSE;
  betting.is_startable =FALSE;
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_destruct
 *  - destruct instance of betting state
 */
void betting_destruct()
{
  state_exit_wait((state_t*)&betting);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_title_lcd (&betting.title_lcd );
  destruct_bet_lcd   (&betting.bet_lcd   );
  destruct_bet_leds  (&betting.bet_leds  );
  destruct_fixed_7seg(&betting.fixed_7seg);
  destruct_single_sw (&betting.single_sw[0]);
  destruct_single_sw (&betting.single_sw[1]);

  state_destruct((state_t*)&betting);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_entry
 *  - implementation for state
 */
void betting_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&betting.unitary);
  betting_construct();
  betting.add_ctx(title_lcd ,&betting.title_lcd );
  betting.add_ctx(bet_lcd   ,&betting.bet_lcd   );
  betting.add_ctx(bet_leds  ,&betting.bet_leds  );
  betting.add_ctx(fixed_7seg,&betting.fixed_7seg);
  betting.add_ctx(single_sw ,&betting.single_sw[0]);

  betting.is_turnable =FALSE;
  if (betting.pcointank->is_turnable())
    betting_begin_wait();

  pthread_mutex_unlock(&betting.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_begin_wait
 *  - message to begin waiting for start-switch down
 */
void betting_begin_wait()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&betting.unitary);
  if (betting.is_turnable == FALSE)
  {
    betting.add_ctx(single_sw ,&betting.single_sw[1]);
    betting.is_turnable =TRUE;
  }
  pthread_mutex_unlock(&betting.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_exit_state
 *  - implementation for state
 */
void betting_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&betting.unitary);
  shutdown_title_lcd (&betting.title_lcd );
  shutdown_bet_lcd   (&betting.bet_lcd   );
  shutdown_bet_leds  (&betting.bet_leds  );
  shutdown_fixed_7seg(&betting.fixed_7seg);
  shutdown_single_sw (&betting.single_sw[0]);
  shutdown_single_sw (&betting.single_sw[1]);
  //betting.is_turnable =FALSE;
  //betting.is_startable =FALSE;//don't touch
  betting.safely_terminated =TRUE;
  pthread_mutex_unlock(&betting.unitary);

  betting_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");
  betting.pcontext->alternate(BETTING,ROLLING_3);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_abort_state
 *  - implementation for state
 */
void betting_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&betting.unitary);
  //betting.is_turnable =FALSE;
  betting.is_startable =FALSE;
  shutdown_title_lcd (&betting.title_lcd );
  shutdown_bet_lcd   (&betting.bet_lcd   );
  shutdown_bet_leds  (&betting.bet_leds  );
  shutdown_fixed_7seg(&betting.fixed_7seg);
  shutdown_single_sw (&betting.single_sw[0]);
  shutdown_single_sw (&betting.single_sw[1]);
  pthread_mutex_unlock(&betting.unitary);

  betting_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * betting_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void betting_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&betting,routine,arg);
}

/**
 * betting_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void betting_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&betting,routine,arg);

  if (arg == &betting.single_sw[1]
   && betting.is_startable ==TRUE
      )
    betting_exit_state();
}
