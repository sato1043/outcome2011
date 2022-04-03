#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(rolling_r);

/**
 * stop_action
 *  - handler for stop-switch down
 */
static int stop_action(single_sw_t* thr)
{
  pthread_mutex_lock(&rolling_r.unitary);

  thr->pstate->preels->fix(thr->index);

  rolling_r.fixed_index =thr->index;

  shutdown_single_sw(thr);

  dbgprint(__FUNCTION__,"reel %d fixed",thr->index);

  pthread_mutex_unlock(&rolling_r.unitary);

  return FALSE;
    //return 0 if you don't want to repeat
}

/**
 * rolling_r
 *  - instance of ROLLING_R state
 */
state_rolling_r_t rolling_r =
{ 
  IMPLEMENT_STATE_BASE(rolling_r)
  IMPLEMENT_TITLE_LCD(rolling_r)
  IMPLEMENT_BET_LCD(rolling_r,1000,100)
  IMPLEMENT_READY_LEDS(rolling_r)
  IMPLEMENT_REACH_7SEG(rolling_r)
  , .fixed_index =-1
  IMPLEMENT_SINGLE_SW(0,rolling_r,"/dev/gpio/swc2",stop_action)
  IMPLEMENT_SINGLE_SW(1,rolling_r,"/dev/gpio/swc1",stop_action)
  IMPLEMENT_SINGLE_SW(2,rolling_r,"/dev/gpio/swc0",stop_action)
};

/**
 * rolling_1_construct
 *  - construt instance of rolling_1 state
 */
void rolling_r_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&rolling_r);
  construct_title_lcd (&rolling_r.title_lcd );
  construct_bet_lcd   (&rolling_r.bet_lcd   );
  construct_ready_leds(&rolling_r.ready_leds);
  construct_reach_7seg(&rolling_r.reach_7seg);
  construct_single_sw (&rolling_r.single_sw[0]);
  construct_single_sw (&rolling_r.single_sw[1]);
  construct_single_sw (&rolling_r.single_sw[2]);
  rolling_r.fixed_index =-1;
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_r_destruct
 *  - destruct instance of rolling_r state
 */
void rolling_r_destruct()
{
  state_exit_wait((state_t*)&rolling_r);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_title_lcd (&rolling_r.title_lcd );
  destruct_bet_lcd   (&rolling_r.bet_lcd   );
  destruct_ready_leds(&rolling_r.ready_leds);
  destruct_reach_7seg(&rolling_r.reach_7seg);
  destruct_single_sw (&rolling_r.single_sw[0]);
  destruct_single_sw (&rolling_r.single_sw[1]);
  destruct_single_sw (&rolling_r.single_sw[2]);

  state_destruct((state_t*)&rolling_r);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_r_entry
 *  - implementation for state
 */
void rolling_r_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_r.unitary);
  rolling_r_construct();
  rolling_r.add_ctx(title_lcd ,&rolling_r.title_lcd );
  rolling_r.add_ctx(bet_lcd   ,&rolling_r.bet_lcd   );
  rolling_r.add_ctx(ready_leds,&rolling_r.ready_leds);
  rolling_r.add_ctx(reach_7seg,&rolling_r.reach_7seg);
  if (rolling_r.preels->not_fixed(0))
    rolling_r.add_ctx(single_sw ,&rolling_r.single_sw[0]);
  if (rolling_r.preels->not_fixed(1))
    rolling_r.add_ctx(single_sw ,&rolling_r.single_sw[1]);
  if (rolling_r.preels->not_fixed(2))
    rolling_r.add_ctx(single_sw ,&rolling_r.single_sw[2]);
  pthread_mutex_unlock(&rolling_r.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_r_exit_state
 *  - implementation for state
 */
void rolling_r_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_r.unitary);
  shutdown_title_lcd (&rolling_r.title_lcd );
  shutdown_bet_lcd   (&rolling_r.bet_lcd   );
  shutdown_ready_leds(&rolling_r.ready_leds);
  shutdown_reach_7seg(&rolling_r.reach_7seg);
  shutdown_single_sw (&rolling_r.single_sw[0]);
  shutdown_single_sw (&rolling_r.single_sw[1]);
  shutdown_single_sw (&rolling_r.single_sw[2]);
  rolling_r.safely_terminated =TRUE;
  pthread_mutex_unlock(&rolling_r.unitary);

  rolling_r_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  if (rolling_r.preels->was_complated())
    rolling_r.pcontext->alternate(ROLLING_R,WINNING);
  else
    rolling_r.pcontext->alternate(ROLLING_R,HOLDING);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_r_abort_state
 *  - implementation for state
 */
void rolling_r_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_r.unitary);
  rolling_r.fixed_index =-1;
  shutdown_title_lcd (&rolling_r.title_lcd );
  shutdown_bet_lcd   (&rolling_r.bet_lcd   );
  shutdown_ready_leds(&rolling_r.ready_leds);
  shutdown_reach_7seg(&rolling_r.reach_7seg);
  shutdown_single_sw (&rolling_r.single_sw[0]);
  shutdown_single_sw (&rolling_r.single_sw[1]);
  shutdown_single_sw (&rolling_r.single_sw[2]);
  pthread_mutex_unlock(&rolling_r.unitary);

  rolling_r_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_r_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_r_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&rolling_r,routine,arg);
}

/**
 * rolling_r_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_r_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&rolling_r,routine,arg);

  single_sw_t* thr =(single_sw_t*)arg;
  if (
     (thr == &rolling_r.single_sw[0]
    ||thr == &rolling_r.single_sw[1]
    ||thr == &rolling_r.single_sw[2])
   && rolling_r.fixed_index == thr->index
      )
    rolling_r_exit_state();
}
