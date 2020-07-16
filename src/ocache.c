#include "ocache.h"
#include "errors.h"
#include <math.h>

#define __INIT_SIZE 8
#define __OFFSET_BASIS 0xcbf29ce484222325
#define __PRIME 0x100000001b3
#define __REGIONS 5

typedef struct region_tag {
  pthread_mutex_t mutex;
  pthread_cond_t write_cond;
  pthread_cond_t read_cond;
  int writers;
  int readers;
} region_t;

//Cache structure
typedef struct cache_tag {
  unsigned long key;
  void* value;
} cache_t;

region_t region[__REGIONS];
cache_t *cache;
int status;

void init() {
  printf("Initializing Cache.\n");
  cache = (cache_t*) malloc(sizeof(cache_t) * __INIT_SIZE);
  for(int i=0;i < __REGIONS;++i) {
    status = pthread_mutex_init(&region[i].mutex, NULL);
    __ERR_REPO(status, "Mutex initialization");

    status = pthread_cond_init(&region[i].write_cond, NULL);
    __ERR_REPO(status, "Condition initialization");

    status = pthread_cond_init(&region[i].read_cond, NULL);
    __ERR_REPO(status, "Condition initialization");
    
    region[i].writers = 0;
    region[i].readers = 0;
  }
  printf("Cache Initialized.\n");
}

void destroy() {
  free(cache);
}

int find_location(uint64_t hash) {
  uint32_t l = (uint32_t) hash;
  uint32_t h = hash >> 32;
  return (l&h) % __INIT_SIZE;
}

unsigned long calculateHash(int key) {
  unsigned long hash = __OFFSET_BASIS;
  
  while (key > 0) {
    hash = hash ^ (key & 0xff);
    hash = hash * __PRIME;
    key = key >> 8;
  }

  return hash;
}

void put(int key, value_t* val) {
  uint64_t hash = calculateHash(key);
  int epr = ceil((double)__INIT_SIZE/(double)__REGIONS);
  int l = find_location(hash);
  int r = l/epr;

  status = pthread_mutex_lock(&region[r].mutex);
  __ERR_REPO(status, "Mutex lock");

  region[r].writers++;
  while (region[r].readers > 0) {
    status = pthread_cond_wait(&region[r].read_cond, &region[r].mutex);
    __ERR_REPO(status, "Waiting for opening of reader gate");
  }

  cache[l].key = key;
  cache[l].value = val;

  region[r].writers--;
  
  status = pthread_cond_signal(&region[r].write_cond);
  __ERR_REPO(status, "Signal opening of writers gate");
  
  status = pthread_mutex_unlock(&region[r].mutex);
  __ERR_REPO(status, "Mutex unlock");
}

void put_if_absent(int key, value_t* val) {
}

value_t *get(int key) {
  value_t* val;
  uint64_t hash = calculateHash(key);
  int epr = ceil((double)__INIT_SIZE/(double)__REGIONS);
  int l = find_location(hash);
  int r = l/epr;

  status = pthread_mutex_lock(&region[r].mutex);
  __ERR_REPO(status, "Mutex lock");

  region[r].readers++;
  while (region[r].writers > 0) {
    status = pthread_cond_wait(&region[r].write_cond, &region[r].mutex);
    __ERR_REPO(status, "Waiting for opening of writer gate");
  }

  val = cache[l].value;

  region[r].readers--;
  
  status = pthread_cond_signal(&region[r].read_cond);
  __ERR_REPO(status, "Signal opening of readers gate");
  
  status = pthread_mutex_unlock(&region[r].mutex);
  __ERR_REPO(status, "Mutex unlock");
  return val;
}

