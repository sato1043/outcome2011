#include <assert.h>//assert
#include <unistd.h>//sleep

#include "thr_glue.h"

#include <pthread.h>

#include "util.h"
#include "state.h"

/**
 * construct_thr_glue
 *  - the function must precede others.
 */
void construct_thr_glue(thr_glue_t* thr)
{
  pthread_rdwr_init_np(&thr->rwlock, NULL);
  thr->is_running =FALSE;
}

/**
 * destruct_thr_glue
 *  - the function must follow others.
 */
void destruct_thr_glue(thr_glue_t* thr)
{
  pthread_rdwr_destroy_np(&thr->rwlock);
}

/**
 * shutdown_thr_glue
 *  - the function set terminate flag for thr_glue thread.
 */
void shutdown_thr_glue(thr_glue_t* thr)
{
  pthread_rdwr_wlock_np(&thr->rwlock);
  if (thr->is_running)
  {
    thr->is_running =FALSE;
  }
  pthread_rdwr_wunlock_np(&thr->rwlock);
}

/**
 * thr_glue
 *  - thread function.
 */
void thr_glue(void *thr_)
{
  assert(thr_);
  title_lcd_t* thr =(title_lcd_t*)thr_;

  pthread_rdwr_wlock_np(&thr->rwlock);
  thr->is_running =TRUE;
  pthread_rdwr_wunlock_np(&thr->rwlock);

  while(thr->is_running)
  {
    usleep(1000);

    shutdown_thr_glue(&rolling_0.thr_glue);
  }

  thr->pstate->rm_ctx(thr_glue,thr_);
}
