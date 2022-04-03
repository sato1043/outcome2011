#include "state.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "reels.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(preparing);

/**
 * preparing
 *  - instance of PREPARING state
 */
state_preparing_t preparing =
{ 
  IMPLEMENT_STATE_BASE(preparing)
  IMPLEMENT_TITLE_LCD(preparing)
  IMPLEMENT_READY_LCD(preparing)
  IMPLEMENT_READY_LEDS(preparing)
  IMPLEMENT_FIXED_7SEG(preparing,"777",500000)
  IMPLEMENT_READY_SW(preparing)
};

/**
 * preparing_construct
 *  - construt instance of preparing state
 */
void preparing_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  state_construct((state_t*)&preparing);
  construct_title_lcd (&preparing.title_lcd );
  construct_ready_lcd (&preparing.ready_lcd );
  construct_ready_leds(&preparing.ready_leds);
  construct_fixed_7seg(&preparing.fixed_7seg);
  construct_ready_sw  (&preparing.ready_sw  );
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * preparing_destruct
 *  - destruct instance of preparing state
 */
void preparing_destruct()
{
  state_exit_wait((state_t*)&preparing);

  dbgprint(__FUNCTION__ ,"BEGIN");

  destruct_title_lcd (&preparing.title_lcd );
  destruct_ready_lcd (&preparing.ready_lcd );
  destruct_ready_leds(&preparing.ready_leds);
  destruct_fixed_7seg(&preparing.fixed_7seg);
  destruct_ready_sw  (&preparing.ready_sw  );

  state_destruct((state_t*)&preparing);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * preparing_entry
 *  - implementation for state
 */
void preparing_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&preparing.unitary);

  preparing_construct();
  preparing_add_ctx(title_lcd ,&preparing.title_lcd );
  preparing_add_ctx(ready_lcd ,&preparing.ready_lcd );
  preparing_add_ctx(ready_leds,&preparing.ready_leds);
  preparing_add_ctx(fixed_7seg,&preparing.fixed_7seg);
  preparing_add_ctx(ready_sw  ,&preparing.ready_sw  );
  pthread_mutex_unlock(&preparing.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * preparing_exit_state
 *  - implementation for state
 */
void preparing_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&preparing.unitary);
  shutdown_title_lcd(&preparing.title_lcd);
  shutdown_ready_lcd(&preparing.ready_lcd);
  shutdown_ready_leds(&preparing.ready_leds);
  shutdown_fixed_7seg(&preparing.fixed_7seg);
  shutdown_ready_sw(&preparing.ready_sw);
  preparing.safely_terminated =TRUE;
  pthread_mutex_unlock(&preparing.unitary);

  preparing_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");
  preparing.pcontext->alternate(PREPARING,BETTING);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * preparing_abort_state
 *  - implementation for state
 */
void preparing_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&preparing.unitary);
  shutdown_title_lcd(&preparing.title_lcd);
  shutdown_ready_lcd(&preparing.ready_lcd);
  shutdown_ready_leds(&preparing.ready_leds);
  shutdown_fixed_7seg(&preparing.fixed_7seg);
  shutdown_ready_sw(&preparing.ready_sw);
  pthread_mutex_unlock(&preparing.unitary);

  preparing_destruct();

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * preparing_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void preparing_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&preparing,routine,arg);
}

/**
 * preparing_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void preparing_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&preparing,routine,arg);
}
