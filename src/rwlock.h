#ifndef __RWLOCK_H_
#define __RW_LOCK_H_

#include <pthread.h>

#define VALID_RW_LOCK 0xcafebabe

/* 
  Interface for read preferred lock. If there are readers waiting to take lock, then they would be preferred over the writer threads.
 */

typedef struct rwlock_tag {
  pthread_mutex_t mutex;
  pthread_contd_t read_allowed;
  pthread_contd_t write_allowed;
  int r_count; //Number of readers
  int w_count; //Number of writers
  int valid; //Indicates if lock is available for use.
} rwlock_t;

/*
  When read lock request is received, it checks if write is already in progress. If so, it waits on the read_allowed condition.
 */
extern void readlock(rwlock_t *lock);

/*
  When read unlock request is received, it checks if there are any readers waiting. If so, broadcasts the read_allowed condition. If there are no readers and writers waiting, signals the write_allowed condition.
 */
extern void readunlock(rwlock_t *lock);

/*
  When write lock request is received, it waits on write_allowed_condition until all readers are done.
 */
extern void writelock(rwlock_t *lock);
/*
  When write unlock request is received, it checks if any readers are waiting.If so, it broadcasts the read_allowed condition. If there are no readers and writers waiting, signals the write_allowed condition.
*/
extern void writeunlock(rwlock_t *lock);
#endif
