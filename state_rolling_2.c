#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(rolling_2);

/**
 * stop_action
 *  - handler for stop-switch down
 */
static int stop_action(single_sw_t* thr)
{
  pthread_mutex_lock(&rolling_2.unitary);

  thr->pstate->preels->fix(thr->index);

  rolling_2.fixed_index =thr->index;

  shutdown_single_sw(thr);

  dbgprint(__FUNCTION__,"reel %d fixed",thr->index);

  pthread_mutex_unlock(&rolling_2.unitary);

  return FALSE;
    //return 0 if you don't want to repeat
}

/**
 * rolling_2
 *  - instance of ROLLING_2 state
 */
state_rolling_2_t rolling_2 =
{ 
  IMPLEMENT_STATE_BASE(rolling_2)
  IMPLEMENT_TITLE_LCD(rolling_2)
  IMPLEMENT_BET_LCD(rolling_2,1000,100)
  IMPLEMENT_READY_LEDS(rolling_2)
  IMPLEMENT_ROLL_7SEG(rolling_2)
  , .fixed_index =-1
  IMPLEMENT_SINGLE_SW(0,rolling_2,"/dev/gpio/swc2",stop_action)
  IMPLEMENT_SINGLE_SW(1,rolling_2,"/dev/gpio/swc1",stop_action)
  IMPLEMENT_SINGLE_SW(2,rolling_2,"/dev/gpio/swc0",stop_action)
};

/**
 * rolling_2_construct
 *  - construt instance of rolling_2 state
 */
void rolling_2_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&rolling_2);
  construct_title_lcd (&rolling_2.title_lcd );
  construct_bet_lcd   (&rolling_2.bet_lcd   );
  construct_ready_leds(&rolling_2.ready_leds);
  construct_roll_7seg (&rolling_2.roll_7seg );
  construct_single_sw (&rolling_2.single_sw[0]);
  construct_single_sw (&rolling_2.single_sw[1]);
  construct_single_sw (&rolling_2.single_sw[2]);
  rolling_2.fixed_index =-1;
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_2_destruct
 *  - destruct instance of rolling_2 state
 */
void rolling_2_destruct()
{
  state_exit_wait((state_t*)&rolling_2);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_title_lcd (&rolling_2.title_lcd );
  destruct_bet_lcd   (&rolling_2.bet_lcd   );
  destruct_ready_leds(&rolling_2.ready_leds);
  destruct_roll_7seg (&rolling_2.roll_7seg );
  destruct_single_sw (&rolling_2.single_sw[0]);
  destruct_single_sw (&rolling_2.single_sw[1]);
  destruct_single_sw (&rolling_2.single_sw[2]);

  state_destruct((state_t*)&rolling_2);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_2_entry
 *  - implementation for state
 */
void rolling_2_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_2.unitary);
  rolling_2_construct();
  rolling_2.add_ctx(title_lcd ,&rolling_2.title_lcd );
  rolling_2.add_ctx(bet_lcd   ,&rolling_2.bet_lcd   );
  rolling_2.add_ctx(ready_leds,&rolling_2.ready_leds);
  rolling_2.add_ctx(roll_7seg ,&rolling_2.roll_7seg );
  if (rolling_2.preels->not_fixed(0))
    rolling_2.add_ctx(single_sw ,&rolling_2.single_sw[0]);
  if (rolling_2.preels->not_fixed(1))
    rolling_2.add_ctx(single_sw ,&rolling_2.single_sw[1]);
  if (rolling_2.preels->not_fixed(2))
    rolling_2.add_ctx(single_sw ,&rolling_2.single_sw[2]);
  pthread_mutex_unlock(&rolling_2.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_2_exit_state
 *  - implementation for state
 */
void rolling_2_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_2.unitary);
  shutdown_title_lcd (&rolling_2.title_lcd );
  shutdown_bet_lcd   (&rolling_2.bet_lcd   );
  shutdown_ready_leds(&rolling_2.ready_leds);
  shutdown_roll_7seg (&rolling_2.roll_7seg );
  shutdown_single_sw (&rolling_2.single_sw[0]);
  shutdown_single_sw (&rolling_2.single_sw[1]);
  shutdown_single_sw (&rolling_2.single_sw[2]);
  rolling_2.safely_terminated =TRUE;
  pthread_mutex_unlock(&rolling_2.unitary);

  rolling_2_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  if (rolling_2.preels->was_reached())
    rolling_2.pcontext->alternate(ROLLING_2,ROLLING_R);
  else
    rolling_2.pcontext->alternate(ROLLING_2,ROLLING_1);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_2_abort_state
 *  - implementation for state
 */
void rolling_2_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_2.unitary);
  rolling_2.fixed_index =-1;
  shutdown_title_lcd (&rolling_2.title_lcd );
  shutdown_bet_lcd   (&rolling_2.bet_lcd   );
  shutdown_ready_leds(&rolling_2.ready_leds);
  shutdown_roll_7seg (&rolling_2.roll_7seg );
  shutdown_single_sw (&rolling_2.single_sw[0]);
  shutdown_single_sw (&rolling_2.single_sw[1]);
  shutdown_single_sw (&rolling_2.single_sw[2]);
  pthread_mutex_unlock(&rolling_2.unitary);

  rolling_2_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_2_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_2_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&rolling_2,routine,arg);
}

/**
 * rolling_2_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_2_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&rolling_2,routine,arg);

  single_sw_t* thr =(single_sw_t*)arg;
  if (
     (thr == &rolling_2.single_sw[0]
    ||thr == &rolling_2.single_sw[1]
    ||thr == &rolling_2.single_sw[2])
   && rolling_2.fixed_index == thr->index
      )
    rolling_2_exit_state();
}
