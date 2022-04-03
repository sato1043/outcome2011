/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * tpool.c -- 
 * 
 * Example thread pooling library
 *
 * mod by satot43@yahoo.co.jp in Jan 6 ,2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include "tpool.h"

#include "util.h"

void *tpool_thread(void *);

void tpool_init(
    tpool_t   *tpoolp,
    int       num_worker_threads, 
    int       max_queue_size,
    int       do_not_block_when_full)
{
  int i, rtn;
  tpool_t tpool;
   
  /* allocate a pool data structure */ 

  if ((tpool =(tpool_t )malloc(sizeof(struct tpool))) == NULL)
  {
    dbgprint(__FUNCTION__,"malloc(1):");
    exit(EXIT_FAILURE);
  }

  /* initialize th fields */

  tpool->num_threads    = num_worker_threads;
  tpool->max_queue_size = max_queue_size;
  tpool->do_not_block_when_full = do_not_block_when_full;
  if ((tpool->threads = 
       (pthread_t *)malloc(sizeof(pthread_t)*num_worker_threads)) 
      == NULL)
  {
    dbgprint(__FUNCTION__,"malloc(2):");
    exit(EXIT_FAILURE);
  }

  tpool->cur_queue_size = 0;
  tpool->queue_head     = NULL; 
  tpool->queue_tail     = NULL;
  tpool->queue_closed   = 0;  
  tpool->shutdown       = 0; 

  if ((rtn = pthread_mutex_init(&(tpool->queue_lock),NULL)) != 0)
  {
    dbgprint(__FUNCTION__,"pthread_mutex_init %s",strerror(rtn));
    exit(EXIT_FAILURE);
  }
  if ((rtn = pthread_cond_init(&(tpool->queue_not_empty),NULL)) != 0)
  {
    dbgprint(__FUNCTION__,"pthread_cond_init %s",strerror(rtn));
    exit(EXIT_FAILURE);
  }
  if ((rtn = pthread_cond_init(&(tpool->queue_not_full),NULL)) != 0)
  {
    dbgprint(__FUNCTION__,"pthread_cond_init %s",strerror(rtn));
    exit(EXIT_FAILURE);
  }
  if ((rtn = pthread_cond_init(&(tpool->queue_empty),NULL)) != 0)
  {
    dbgprint(__FUNCTION__,"pthread_cond_init %s",strerror(rtn));
    exit(EXIT_FAILURE);
  }

  /* create threads */

  for (i = 0; i != num_worker_threads; i++)
  {
    if ((rtn = pthread_create( &(tpool->threads[i]),
                              NULL,
                              tpool_thread,
                              (void *)tpool)) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_create %d (worker=%d)",rtn,i);
      exit(EXIT_FAILURE);
    }
  }

  *tpoolp = tpool;
}

int tpool_add_work(
    tpool_t   tpool
  , void      (*routine)(void *)
  , void      *arg)
{
#define ADDITION_SUCCEED  (0)
#define ADDITION_TIMEOUT  (-1)
#define ADDITION_CANCELED (-2)

  int rtn;

  tpool_work_t *workp;

  if ((rtn =pthread_mutex_lock(&(tpool->queue_lock))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_mutex_lock %d",rtn);
    exit(EXIT_FAILURE);
  }

  /* no space and this caller doesn't want to wait */

  if (tpool->cur_queue_size == tpool->max_queue_size
   && tpool->do_not_block_when_full)
  {
    if ((rtn =pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
      exit(EXIT_FAILURE);
    }

    return ADDITION_TIMEOUT;
  }

  while( tpool->cur_queue_size == tpool->max_queue_size
      && (!(tpool->shutdown || tpool->queue_closed)) )
  {
    if ((rtn = pthread_cond_wait(&(tpool->queue_not_full),
                                 &(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_cond_wait %d",rtn);
      exit(EXIT_FAILURE);
    }
  }

  /* the pool is in the process of being destroyed */

  if (tpool->shutdown || tpool->queue_closed)
  {
    if ((rtn =pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
      exit(EXIT_FAILURE);
    }
 
    return ADDITION_CANCELED;
  }

  /* allocate work structure */

  if ((workp = (tpool_work_t *)malloc(sizeof(tpool_work_t))) == NULL)
  {
    dbgprint(__FUNCTION__,"malloc tpool_work_t:");
    exit(EXIT_FAILURE);
  }

  workp->routine = routine;
  workp->arg     = arg;
  workp->next    = NULL;

  dbgprint(__FUNCTION__ ,"adder: adding an item %p with arg %p"
      ,workp->routine ,workp->arg);

  if (tpool->cur_queue_size == 0)
  {
    tpool->queue_tail =tpool->queue_head =workp;

    //dbgprint(__FUNCTION__ ,"adder: queue == 0, waking all workers");

    if ((rtn =pthread_cond_broadcast(&(tpool->queue_not_empty))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_cond_broadcast %d",rtn);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    tpool->queue_tail->next =workp;
    tpool->queue_tail       =workp;
  }

  tpool->cur_queue_size++; 

  if ((rtn =pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
    exit(EXIT_FAILURE);
  }

  return ADDITION_SUCCEED;

#undef ADDITION_SUCCEED  
#undef ADDITION_TIMEOUT
#undef ADDITION_CANCELED
}

int tpool_destroy(
    tpool_t   tpool
  , int       finish
)
{
  int          i,rtn;
  tpool_work_t *cur_nodep;
  

  if ((rtn =pthread_mutex_lock(&(tpool->queue_lock))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_mutex_lock %d",rtn);
    exit(EXIT_FAILURE);
  }

  /* Is a shutdown already in progress? */

  if (tpool->queue_closed || tpool->shutdown)
  {
    if ((rtn =pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
      exit(EXIT_FAILURE);
    }
    return 0;
  }

  tpool->queue_closed = 1;

  /* If the finish flag is set, wait for workers to 
     drain queue */ 
  if (finish == 1)
  {
    while (tpool->cur_queue_size != 0)
    {
      if ((rtn =pthread_cond_wait(&(tpool->queue_empty),
                                  &(tpool->queue_lock))) != 0)
      {
        dbgprint(__FUNCTION__ ,"pthread_cond_wait %d",rtn);
        exit(EXIT_FAILURE);
      }
    }
  }

  tpool->shutdown = 1;

  if ((rtn =pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
    exit(EXIT_FAILURE);
  }


  /* Wake up any workers so they recheck shutdown flag */

  if ((rtn =pthread_cond_broadcast(&(tpool->queue_not_empty))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_cond_broadcast %d",rtn);
    exit(EXIT_FAILURE);
  }
  if ((rtn =pthread_cond_broadcast(&(tpool->queue_not_full))) != 0)
  {
    dbgprint(__FUNCTION__ ,"pthread_cond_broadcast %d",rtn);
    exit(EXIT_FAILURE);
  }


  /* Wait for workers to exit */

  for(i=0; i < tpool->num_threads; i++)
  {
    if ((rtn =pthread_join(tpool->threads[i],NULL)) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_join %d",rtn);
      exit(EXIT_FAILURE);
    }
  }

  /* Now free pool structures */

  free(tpool->threads);

  while(tpool->queue_head != NULL)
  {
    cur_nodep         = tpool->queue_head->next; 
    tpool->queue_head = tpool->queue_head->next;
    free(cur_nodep);
  }
  free(tpool); 

  return 0;
}

void *tpool_thread(void *arg)
{
  tpool_t tpool = (tpool_t)arg; 
  int rtn;
  tpool_work_t *my_workp;
        
  for(;;)
  {

    /* Check queue for work */ 

    if ((rtn =pthread_mutex_lock(&(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_mutex_lock %d",rtn);
      exit(EXIT_FAILURE);
    }

    while ((tpool->cur_queue_size == 0) && (!tpool->shutdown))
    {
      //dbgprint(__FUNCTION__ ,"worker %d: I'm sleeping again", pthread_self());

      if ((rtn = pthread_cond_wait(&(tpool->queue_not_empty),
                                   &(tpool->queue_lock))) != 0)
      {
        dbgprint(__FUNCTION__ ,"pthread_cond_wait %d",rtn);
        exit(EXIT_FAILURE);
      }
    }

    //sleep(5); 
 
    //dbgprint(__FUNCTION__ ,"worker %d: I'm awake", pthread_self());

    /* Has a shutdown started while i was sleeping? */

    if (tpool->shutdown == 1)
    {
      if ((rtn = pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
      {
        dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
        exit(EXIT_FAILURE);
      }

      dbgprint(__FUNCTION__ ,"worker %08lx: I dead." ,pthread_self());

      pthread_exit(NULL);
    }

    /* Get to work, dequeue the next item */ 

    my_workp = tpool->queue_head;

    tpool->cur_queue_size--;

    if (tpool->cur_queue_size == 0)
    {
      tpool->queue_head =tpool->queue_tail =NULL;
    }
    else
    {
      tpool->queue_head =my_workp->next;
    }
 
    dbgprint(__FUNCTION__ ,"worker %08lx: dequeing item %p with arg %p"
        ,pthread_self() ,my_workp->routine ,my_workp->arg);

    /* Handle waiting add_work threads */

    if ((!tpool->do_not_block_when_full) &&
        (tpool->cur_queue_size ==  (tpool->max_queue_size - 1))) 
    {
      if ((rtn = pthread_cond_broadcast(&(tpool->queue_not_full))) != 0)
      {
        dbgprint(__FUNCTION__ ,"pthread_cond_broadcast %d",rtn);
        exit(EXIT_FAILURE);
      }
    }

    /* Handle waiting destroyer threads */

    if (tpool->cur_queue_size == 0)
    {
      if ((rtn = pthread_cond_signal(&(tpool->queue_empty))) != 0)
      {
        dbgprint(__FUNCTION__ ,"pthread_cond_signal %d",rtn);
        exit(EXIT_FAILURE);
      }
    }

    if ((rtn = pthread_mutex_unlock(&(tpool->queue_lock))) != 0)
    {
      dbgprint(__FUNCTION__ ,"pthread_mutex_unlock %d",rtn);
      exit(EXIT_FAILURE);
    }
      
    /* Do this work item */

    (*(my_workp->routine))(my_workp->arg);

    /* free this work item */

    free(my_workp);

  }// for(;;)

  return(NULL);            
}



