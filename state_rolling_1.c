#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(rolling_1);

/**
 * stop_action
 *  - handler for stop-switch down
 */
static int stop_action(single_sw_t* thr)
{
  pthread_mutex_lock(&rolling_1.unitary);

  thr->pstate->preels->fix(thr->index);

  rolling_1.fixed_index =thr->index;

  shutdown_single_sw(thr);

  dbgprint(__FUNCTION__,"reel %d fixed",thr->index);

  pthread_mutex_unlock(&rolling_1.unitary);

  return FALSE;
    //return 0 if you don't want to repeat
}

/**
 * rolling_1
 *  - instance of ROLLING_1 state
 */
state_rolling_1_t rolling_1 =
{ 
  IMPLEMENT_STATE_BASE(rolling_1)
  IMPLEMENT_TITLE_LCD(rolling_1)
  IMPLEMENT_BET_LCD(rolling_1,1000,100)
  IMPLEMENT_READY_LEDS(rolling_1)
  IMPLEMENT_ROLL_7SEG(rolling_1)
  , .fixed_index =-1
  IMPLEMENT_SINGLE_SW(0,rolling_1,"/dev/gpio/swc2",stop_action)
  IMPLEMENT_SINGLE_SW(1,rolling_1,"/dev/gpio/swc1",stop_action)
  IMPLEMENT_SINGLE_SW(2,rolling_1,"/dev/gpio/swc0",stop_action)
};

/**
 * rolling_1_construct
 *  - construt instance of rolling_1 state
 */
void rolling_1_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&rolling_1);
  construct_title_lcd (&rolling_1.title_lcd );
  construct_bet_lcd   (&rolling_1.bet_lcd   );
  construct_ready_leds(&rolling_1.ready_leds);
  construct_roll_7seg (&rolling_1.roll_7seg );
  construct_single_sw (&rolling_1.single_sw[0]);
  construct_single_sw (&rolling_1.single_sw[1]);
  construct_single_sw (&rolling_1.single_sw[2]);
  rolling_1.fixed_index =-1;
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_1_destruct
 *  - destruct instance of rolling_1 state
 */
void rolling_1_destruct()
{
  state_exit_wait((state_t*)&rolling_1);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_title_lcd (&rolling_1.title_lcd );
  destruct_bet_lcd   (&rolling_1.bet_lcd   );
  destruct_ready_leds(&rolling_1.ready_leds);
  destruct_roll_7seg (&rolling_1.roll_7seg );
  destruct_single_sw (&rolling_1.single_sw[0]);
  destruct_single_sw (&rolling_1.single_sw[1]);
  destruct_single_sw (&rolling_1.single_sw[2]);

  state_destruct((state_t*)&rolling_1);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_1_entry
 *  - implementation for state
 */
void rolling_1_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_1.unitary);
  rolling_1_construct();
  rolling_1.add_ctx(title_lcd ,&rolling_1.title_lcd );
  rolling_1.add_ctx(bet_lcd   ,&rolling_1.bet_lcd   );
  rolling_1.add_ctx(ready_leds,&rolling_1.ready_leds);
  rolling_1.add_ctx(roll_7seg ,&rolling_1.roll_7seg );
  if (rolling_1.preels->not_fixed(0))
    rolling_1.add_ctx(single_sw ,&rolling_1.single_sw[0]);
  if (rolling_1.preels->not_fixed(1))
    rolling_1.add_ctx(single_sw ,&rolling_1.single_sw[1]);
  if (rolling_1.preels->not_fixed(2))
    rolling_1.add_ctx(single_sw ,&rolling_1.single_sw[2]);
  pthread_mutex_unlock(&rolling_1.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_1_exit_state
 *  - implementation for state
 */
void rolling_1_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_1.unitary);
  shutdown_title_lcd (&rolling_1.title_lcd );
  shutdown_bet_lcd   (&rolling_1.bet_lcd   );
  shutdown_ready_leds(&rolling_1.ready_leds);
  shutdown_roll_7seg (&rolling_1.roll_7seg );
  shutdown_single_sw (&rolling_1.single_sw[0]);
  shutdown_single_sw (&rolling_1.single_sw[1]);
  shutdown_single_sw (&rolling_1.single_sw[2]);
  rolling_1.safely_terminated =TRUE;
  pthread_mutex_unlock(&rolling_1.unitary);

  rolling_1_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  if (rolling_1.preels->was_reached())
    rolling_1.pcontext->alternate(ROLLING_1,HOLDING);
  else
    rolling_1.pcontext->alternate(ROLLING_1,ROLLING_0);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_1_abort_state
 *  - implementation for state
 */
void rolling_1_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_1.unitary);
  rolling_1.fixed_index =-1;
  shutdown_title_lcd (&rolling_1.title_lcd );
  shutdown_bet_lcd   (&rolling_1.bet_lcd   );
  shutdown_ready_leds(&rolling_1.ready_leds);
  shutdown_roll_7seg (&rolling_1.roll_7seg );
  shutdown_single_sw (&rolling_1.single_sw[0]);
  shutdown_single_sw (&rolling_1.single_sw[1]);
  shutdown_single_sw (&rolling_1.single_sw[2]);
  pthread_mutex_unlock(&rolling_1.unitary);

  rolling_1_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_1_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_1_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&rolling_1,routine,arg);
}

/**
 * rolling_1_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_1_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&rolling_1,routine,arg);

  single_sw_t* thr =(single_sw_t*)arg;
  if (
     (thr == &rolling_1.single_sw[0]
    ||thr == &rolling_1.single_sw[1]
    ||thr == &rolling_1.single_sw[2])
   && rolling_1.fixed_index == thr->index
      )
    rolling_1_exit_state();
}
