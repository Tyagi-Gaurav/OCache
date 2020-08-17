#include "rwlock.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

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
  if (status != 0) return status;

  //Mark the lock as invalid.
  lock->valid = 0;
  
  while (lock->r_count > 0) {
    //Wait for readers to finish.
    status = pthread_cond_wait(&lock->read_allowed, &lock->mutex);
    if (status != 0) return status;
  }

  while (lock->w_count > 0) {
    //Wait for writers to finish.
    status = pthread_cond_wait(&lock->write_allowed, &lock->mutex);
    if (status != 0) return status;
  }

  status = pthread_mutex_unlock(&lock->mutex);
  if (status != 0) return status;

  pthread_cond_destroy(&lock->read_allowed);
  pthread_cond_destroy(&lock->write_allowed);
  pthread_mutex_destroy(&lock->mutex);
  return 0;
}

int readlock(rwlock_t *lock) {
  int status;
  if (lock->valid != VALID_RW_LOCK)
    return EINVAL;

  status = pthread_mutex_lock(&lock->mutex);
  if (status != 0) return status;

  //Register interest for reading.
  lock->r_count++;
  
  while (lock->w_count > 0) {
    //There is exclusive write lock on the cache.
    //Wait until write finishes its work.
    status = pthread_cond_wait(&lock->read_allowed, &lock->mutex);
    if (status != 0) {
      pthread_mutex_unlock(&lock->mutex);
      return status;
    }
  }
  
  status = pthread_mutex_unlock(&lock->mutex);
  return status;
}

int writelock(rwlock_t *lock) {
  int status;

  if (lock->valid != VALID_RW_LOCK)
    return EINVAL;

  status = pthread_mutex_lock(&lock->mutex);
  if (status != 0) return status;

  /*
    Check if previous writers have taken lock. 
    If so, wait for them to finish.
  */
  while (lock->w_count > 0) {
    status = pthread_cond_wait(&lock->write_allowed, &lock->mutex);
    if (status != 0) {
      pthread_mutex_unlock(&lock->mutex);
      return status;
    }
  }
  
  lock->w_count++;

  /*
    Check if previous readers have taken lock. 
    If so, wait for them to finish.
  */
  while (lock->r_count > 0) {
    status = pthread_cond_wait(&lock->write_allowed, &lock->mutex);
    if (status != 0) {
      pthread_mutex_unlock(&lock->mutex);
      return status;
    }
  }

  return pthread_mutex_unlock(&lock->mutex);
}

int readunlock(rwlock_t *lock) {
  int status, status2 = 0;

  if (lock->valid != VALID_RW_LOCK)
    return EINVAL;

  status = pthread_mutex_lock(&lock->mutex);
  if (status != 0) return status;

  lock->r_count--;

  if (lock->r_count > 0) {
    status = pthread_mutex_unlock(&lock->mutex);
    return status;
  }

  if (lock->w_count > 0) {
    status2 = pthread_cond_signal(&lock->write_allowed);
  }
  
  return status2 + pthread_mutex_unlock(&lock->mutex);
}

int writeunlock(rwlock_t *lock) {
  int status, status2 = 0;

  if (lock->valid != VALID_RW_LOCK) 
    return EINVAL;

  status = pthread_mutex_lock(&lock->mutex);
  if (status != 0) return status;

  lock->w_count--;

  if (lock->r_count > 0) {
    status2 = pthread_cond_broadcast(&lock->read_allowed);
    if (status2 != 0) {
      return pthread_mutex_unlock(&lock->mutex);
    }
  }

  if (lock->w_count > 0) {
    status2 = pthread_cond_signal(&lock->write_allowed);
    if (status2 != 0) {
      return pthread_mutex_unlock(&lock->mutex);
    }
  }

  return pthread_mutex_unlock(&lock->mutex);
}
