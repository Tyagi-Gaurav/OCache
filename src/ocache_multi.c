#include "ocache.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 3

typedef struct args_tag {
  int thread_id;
  int seed;
}args_t;


pthread_once_t once_block = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;
int success;
int error;

void once_init_routine(void) {
  int status;

  status = pthread_mutex_init(&mutex, NULL);
  __ERR_REPO(status, "Init Mutex");

  init();
}

void *thread_routine(void* args) {
  int status;  
  status = pthread_once(&once_block, once_init_routine);
  
  args_t *i_args = (args_t*)args;
  int id = pthread_self(); //i_args->thread_id;
  srand(i_args->seed + 86400 * id);
  
  while (1) {
    pthread_testcancel();
    
    //Get random number
    int r = rand();
    
    //Create Value
    time_t exp_ttl = time(NULL) + r;
    value_t *val = malloc(sizeof(val));
    val->data = (int *)24;
    val->ttl = exp_ttl;

    //Put it into cache
    printf("Thread %d:  value adding %d with TTL: %ld\n", id, r, exp_ttl);
    put(r, val);

    //Sleep for x seconds
    sleep(1);

    //get the value back from cache
    value_t *actual = get(r);

    //compare they are equal
    if (actual == NULL) {
      printf("Thread %d: Actual pointer is null for key %d\n", id, r);
      abort();
    }

    if (actual->data != (int*)24) {
      printf("Thread %d: Data does not match\n", id);
      abort();
    }
    
    if (actual->ttl != exp_ttl) {
      printf("Thread %d: TTL does not match. Actual: %ld, exp: %ld\n", id, actual->ttl, exp_ttl);
      abort();
    }

    free(val);
  }
  
  return 0;
}

int main(int argc, char **argv) {
  pthread_t thread_id[MAX_THREADS];
  int status;
  time_t t;
  status = pthread_once(&once_block, once_init_routine);
  __ERR_REPO(status, "Once init");

  srand((unsigned)time(&t));
  
  for (int i=0;i < MAX_THREADS;++i) {
    args_t args = {i+1, rand()};
    status = pthread_create(&thread_id[i], NULL, thread_routine, (void*)&args);
    sleep(4);
    __ERR_REPO(status, "Create thread");
  }

  //Wait for duration of the test and then stop
  sleep(60); //Seconds

  //Stop the threads
  for (int i=0;i < MAX_THREADS;++i) {
    status = pthread_cancel(thread_id[i]);
    __ERR_REPO(status, "Cancel thread");
  }

  //Wait for threads to finish
  for (int i=0;i < MAX_THREADS;++i) {     
    status = pthread_join(thread_id[i], NULL);
    __ERR_REPO(status, "Waiting for thread");
  }
  
  return 0;  
}
