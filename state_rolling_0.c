#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

DECLARE_STATE_BASE_METHOD(rolling_0);

/**
 * rolling_0
 *  - instance of ROLLING_0 state
 */
state_rolling_0_t rolling_0 =
{ 
  IMPLEMENT_STATE_BASE(rolling_0)
  IMPLEMENT_THR_GLUE(rolling_0)
};

/**
 * rolling_0_construct
 *  - construt instance of rolling_0 state
 */
void rolling_0_construct()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  state_construct((state_t*)&rolling_0);
  construct_thr_glue (&rolling_0.thr_glue );
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_0_destruct
 *  - destruct instance of rolling_0 state
 */
void rolling_0_destruct()
{
  state_exit_wait((state_t*)&rolling_0);

  dbgprint(__FUNCTION__ ,"BEGIN");
  destruct_thr_glue (&rolling_0.thr_glue );

  state_destruct((state_t*)&rolling_0);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_0_entry
 *  - implementation for state
 */
void rolling_0_entry()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_0.unitary);
  rolling_0_construct();
  rolling_0.add_ctx(thr_glue ,&rolling_0.thr_glue );
  pthread_mutex_unlock(&rolling_0.unitary);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_0_exit_state
 *  - implementation for state
 */
void rolling_0_exit_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");

  pthread_mutex_lock(&rolling_0.unitary);
  shutdown_thr_glue (&rolling_0.thr_glue );
  rolling_0.safely_terminated =TRUE;
  pthread_mutex_unlock(&rolling_0.unitary);

  rolling_0_destruct();

  dbgprint(__FUNCTION__ ,"ALTERNATE");

  if (rolling_0.pplayer->can_play())
    rolling_0.pcontext->alternate(ROLLING_0,BETTING);
  else
    rolling_0.pcontext->alternate(ROLLING_0,PREPARING);

  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_0_abort_state
 *  - implementation for state
 */
void rolling_0_abort_state()
{
  dbgprint(__FUNCTION__ ,"BEGIN");
  pthread_mutex_lock(&rolling_0.unitary);
  //nothing
  pthread_mutex_unlock(&rolling_0.unitary);
  dbgprint(__FUNCTION__ ,"END");
}

/**
 * rolling_0_add_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_0_add_ctx(void (*routine)(void *) ,void *arg) //call under lock
{
  state_add_ctx((state_t*)&rolling_0,routine,arg);
}

/**
 * rolling_0_rm_ctx
 *  - implementation for state
 *
 * @param routine thread function
 * @param arg thread's argument
 */
void rolling_0_rm_ctx(void (*routine)(void *) ,void *arg)
{
  state_rm_ctx((state_t*)&rolling_0,routine,arg);

  rolling_0_exit_state();
}
