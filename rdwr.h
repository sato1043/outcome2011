#ifndef __rdwr_h__
#define __rdwr_h__

/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * rdwr.h --
 * 
 * Include file for reader/writer locks
 */
/**
 * mod by satot43@yahoo.co.jp Jan 7,2010
 */

#include <pthread.h>

typedef struct pthread_rdwr_t_ {
  int readers_reading;
  int writer_writing;
  pthread_mutex_t mutex;
  pthread_cond_t lock_free;
  int was_destructed;
} pthread_rdwr_t;

typedef void * pthread_rdwrattr_t;

#define pthread_rdwrattr_default NULL;

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp);
int pthread_rdwr_destroy_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp);

#endif/*__rdwr_h__*/
