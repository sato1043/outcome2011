#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

/**
 * get_state_instance
 *  - returns instance of state
 *
 * @param type state type to retrieve
 * @return pointer to the relataed state's instance
 */
state_t* get_state_instance(state_type_t type)
{
  switch(type)
  {
    case  NEUTRAL   : return (state_t*)NULL       ;
    case  PREPARING : return (state_t*)&preparing ;
    case  BETTING   : return (state_t*)&betting   ;
    case  ROLLING_3 : return (state_t*)&rolling_3 ;
    case  ROLLING_2 : return (state_t*)&rolling_2 ;
    case  ROLLING_1 : return (state_t*)&rolling_1 ;
    case  ROLLING_0 : return (state_t*)&rolling_0 ;
    case  ROLLING_R : return (state_t*)&rolling_r ;
    case  HOLDING   : return (state_t*)&holding   ;
    case  WINNING   : return (state_t*)&winning   ;
    default: break;
  }
  return 0;
}

/**
 * state_construct
 *  - construct state
 *  - base class constructer
 *
 * @param pstate pointer to construct
 */
void state_construct(state_t* pstate)
{
  pthread_mutex_init(&pstate->unitary,NULL);
#if 0
  pstate->this       =pstate;
  pstate->preels     =&reels;
  pstate->pplayer    =&player;
  pstate->pcontext   =&gamectx;
  pstate->pcointank  =&cointank;
  pstate->psettings  =&settings;
  pstate->construct  =construct;
  pstate->destruct   =destruct;
  pstate->entry      =entry;
  pstate->exit_state =exit_state;
  pstate->abort_state=abort_state;
  pstate->add_ctx    =add_ctx;
  pstate->rm_ctx     =rm_ctx;
#endif
  pstate->thread_ref_cnt =0;
  pthread_cond_init(&pstate->no_more_thr,NULL);
  pstate->safely_terminated =FALSE;
}

/**
 * state_add_ctx
 *  - add a work to the state
 *
 * @param pstate pointer to construct
 * @param thread routine
 * @param arg thread's argument
 */
void state_add_ctx(state_t* pstate ,void (*routine)(void *) ,void *arg)
{
  // call under lock

  ++pstate->thread_ref_cnt;

  pstate->pcontext->add_ctx(routine,arg);

  dbgprint(__FUNCTION__ ,"routine=%p arg=%p",routine,arg);
}

/**
 * state_rm_ctx
 *  - remove a work from the state
 *
 * @param pstate pointer to construct
 * @param thread routine
 * @param arg thread's argument
 */
void state_rm_ctx(state_t* pstate ,void (*routine)(void *) ,void *arg)
{
  dbgprint(__FUNCTION__ ,"routine=%p arg=%p",routine,arg);

  pthread_mutex_lock(&pstate->unitary);

  --pstate->thread_ref_cnt;

  pthread_cond_signal(&pstate->no_more_thr);

  pthread_mutex_unlock(&pstate->unitary);
}

/**
 * state_exit_wait
 *  - wait for quiting all of state's works
 *
 * @param pstate pointer to construct
 */
void state_exit_wait(state_t* pstate)
{
  pthread_mutex_lock(&pstate->unitary);

  dbgprint(__FUNCTION__ ,"(pstate=%p ref_cnt=%d) BEGIN",pstate,pstate->thread_ref_cnt);

  while(pstate->thread_ref_cnt)
  {
    pthread_cond_wait(&pstate->no_more_thr,&pstate->unitary);
    dbgprint(__FUNCTION__ ,"pstate->thread_ref_cnt=%d",pstate->thread_ref_cnt);
  }

  dbgprint(__FUNCTION__ ,"(pstate=%p) succeded.",pstate);

  pthread_mutex_unlock(&pstate->unitary);
}

/**
 * state_destruct
 *  - destruct state
 */
void state_destruct(state_t* pstate)
{
  pthread_cond_destroy(&pstate->no_more_thr);
  pthread_mutex_destroy(&pstate->unitary);
}
