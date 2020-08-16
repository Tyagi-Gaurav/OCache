#include "rwlock.h"
#include <stdlib.h>
#include <errno.h>

int init_lock(rwlock_t *lock) {
  int status;
  lock->valid = 0;

  status = pthread_mutex_init(&lock->mutex, NULL);
  if (status != 0) return status;

  status = pthread_cond_init(&lock->read_allowed, NULL);
  if (status != 0) {
    pthread_mutex_destroy(&lock->mutex);
    return status;
  }

  status = pthread_cond_init(&lock->write_allowed, NULL);
  if (status != 0) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->read_allowed);
    return status;
  }

  lock->r_count = 0;
  lock->w_count = 0;
  lock->valid = VALID_RW_LOCK;
  return 0;
}

int destroy_lock(rwlock_t *lock) {
  int status;
  
  if (lock->valid != VALID_RW_LOCK)
    return EINVAL;

  status = pthread_mutex_lock(&lock->mutex);
  if (status != 0)
    return status;

  //Mark the lock as invalid.
  lock->valid = 0;
  
  while (lock->r_count > 0) {
    //Wait for readers to finish.
    status = pthread_cond_wait(&lock->read_allowed, &lock->mutex);
    if (status != 0) return status;
  }

  while (lock->w_count > 0) {
    //Wait for readers to finish.
    status = pthread_cond_wait(&lock->write_allowed, &lock->mutex);
    if (status != 0) return status;
  }

  return 0;
}
