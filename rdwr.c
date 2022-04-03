/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * rdwr.c --
 * 
 * Library of functions implementing reader/writer locks
 */
/**
 * mod by satot43@yahoo.co.jp Jan 7,2010
 */
#include <pthread.h>
#include "rdwr.h"

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp)
{
  rdwrp->readers_reading =0;
  rdwrp->writer_writing  =0;
  rdwrp->was_destructed  =0;
  // DO NOT USE recursive lock with condition variables. see man page of pthread_mutexattr_settype.
  pthread_mutex_init(&(rdwrp->mutex),NULL);
  pthread_cond_init(&(rdwrp->lock_free), NULL);
  return 0;
}

int pthread_rdwr_destroy_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  while(rdwrp->writer_writing || rdwrp->readers_reading)
  {
    //there're any user for this lock then,
    pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex));
      //unlock and sleep until no other exists.
      //read-unlock or write-lock context tells that.
      //then wake up and lock
  }
  rdwrp->was_destructed  =1;
  pthread_mutex_unlock(&(rdwrp->mutex));

  pthread_cond_destroy(&(rdwrp->lock_free));
  pthread_mutex_destroy(&(rdwrp->mutex));
  return 0;
}

/**
 * pthread_rdwr_rlock_np
 *  - the function waits
 *    until a calling context become to be able to read.
 */
int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  if (rdwrp->was_destructed)
    return -1;

  //noone is writing, everyone can get a read-lock.
  //someone is writing, wait a while
  while(rdwrp->writer_writing)
  {
    pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex));
      //unlock and sleep, if signal recieved then wake up and lock
  }
  rdwrp->readers_reading++;
  pthread_mutex_unlock(&(rdwrp->mutex));
  return 0;
}

/**
 * pthread_rdwr_runlock_np
 *  - the function finish calling context to be able to read.
 *    and tell a waiting context(readers or a writer)
 *     to become those turn.
 */
int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  if (rdwrp->was_destructed)
    return -1;

  if (rdwrp->readers_reading == 0)
  {
    //from the begining, noone has a lock.
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  }
  else
  {
    //a calling context finish using a lock.
    rdwrp->readers_reading--;
    if (rdwrp->readers_reading == 0)
    {
      //if there's no other reader then,
      pthread_cond_signal(&(rdwrp->lock_free));
        //tell that to a waiting context.
    }
    pthread_mutex_unlock(&(rdwrp->mutex));
    return 0;
  }
}

/**
 * pthread_rdwr_wlock_np
 *  - the function waits
 *    until a calling context become to be able to write.
 */
int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  if (rdwrp->was_destructed)
    return -1;

  //only one context can get a write-lock at a time
  //since a resouce rewriting is destructive 
  //therefore noone can get a read-lock while it's used for writing.
  //so we must wait and sleep while someone has a lock.
  while(rdwrp->writer_writing || rdwrp->readers_reading)
  {
    //there're any user for this lock then,
    pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex));
      //unlock and sleep until no other exists.
      //read-unlock or other write-lock context tells that.
      //then wake up and lock.
  }
  rdwrp->writer_writing++;
  pthread_mutex_unlock(&(rdwrp->mutex));
  return 0;
}

/**
 * pthread_rdwr_wunlock_np
 *  - the function finish calling context to be able to write.
 *    and tell a waiting contexts(readers or a writer)
 *     to become those turn.
 */
int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp)
{
  pthread_mutex_lock(&(rdwrp->mutex));
  if (rdwrp->was_destructed)
    return -1;

  if (rdwrp->writer_writing == 0)
  {
    //from the begining, noone has a lock.
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  }
  else
  {
    //a calling context finish using a write-lock.
    rdwrp->writer_writing = 0;
    pthread_cond_broadcast(&(rdwrp->lock_free));
      //tell every lock-waiting context that.
    pthread_mutex_unlock(&(rdwrp->mutex));
    return 0;
  }
}
