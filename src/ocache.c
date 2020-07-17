

#include "ocache.h"
#include "errors.h"
#include <math.h>

#define __INIT_SIZE 2
#define __OFFSET_BASIS 0xcbf29ce484222325
#define __PRIME 0x100000001b3
#define __REGIONS 5

typedef struct node_tag {
  unsigned long key;
  void* value;
  struct node_tag *next;
} node_t;

//Cache structure
typedef struct cache_entry_tag {
  node_t *node;
  node_t *end;
} cache_entry_t;

typedef struct region_tag {
  pthread_mutex_t mutex;
  pthread_cond_t write_cond;
  cache_entry_t *cache;
  int c_size;
  int t_size;
} region_t;

region_t region[__REGIONS];
int status;

void init() {
  printf("Initializing Cache.\n");
  for(int i=0;i < __REGIONS;++i) {
    status = pthread_mutex_init(&region[i].mutex, NULL);
    __ERR_REPO(status, "Mutex initialization");

    status = pthread_cond_init(&region[i].write_cond, NULL);
    __ERR_REPO(status, "Condition initialization");

    region[i].cache = malloc(sizeof(cache_entry_t) * __INIT_SIZE);
    memset(region[i].cache, 0, sizeof(cache_entry_t) * __INIT_SIZE);
    
    region[i].c_size = 0;
    region[i].t_size = __INIT_SIZE;
  }
  printf("Cache Initialized.\n");
}

void destroy() {
  
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
  int r = hash % 5; //region
  int l = find_location(hash);
  region_t reg = region[r];
  
  status = pthread_mutex_lock(&reg.mutex);
  __ERR_REPO(status, "Mutex lock");

  if (reg.c_size+1 < reg.t_size) {
    //Adding key to node for the first time.
    if (reg.cache[l].node == NULL) {
      reg.cache[l].node = malloc(sizeof(node_t));
      reg.cache[l].node->key = key;
      reg.cache[l].node->value = val;
      reg.cache[l].node->next = NULL;
      reg.cache[l].end = reg.cache[l].node;
    } else {
      //There's already a value at the location. Add to linked list.
      #ifdef __DEBUG
      printf("Place already taken by key: %lu\n", reg.cache[l].node->key);
      #endif
      node_t *new_nd = malloc(sizeof(node_t));
      new_nd->next = NULL;
      new_nd->key = key;
      new_nd->value = val;
      reg.cache[l].end->next = new_nd;
      reg.cache[l].end = new_nd;
    }
    
    reg.c_size++;
  } else 
    printf("Cache full.\n");

  #ifdef __DEBUG
  printf("Added key %d to region:location: %d:%d and size is now: %d\n", key,r, l, reg.c_size);
  #endif
  
  status = pthread_cond_signal(&reg.write_cond);
  __ERR_REPO(status, "Signal opening of writers gate");
  
  status = pthread_mutex_unlock(&reg.mutex);
  __ERR_REPO(status, "Mutex unlock");
}

void put_if_absent(int key, value_t* val) {
  
}

value_t *get(int key) {
  uint64_t hash = calculateHash(key);
  int r = hash % 5; //region
  int l = find_location(hash);

  node_t *n = region[r].cache[l].node;
  while (n != NULL && n->key != key) n = n->next;

  if (n == NULL)
    return NULL;
  else
    return n->value;
}

/*
TODO
====
* During get search for all the keys in the linked list.
* Dynamically increase the cache size per region.
* If element not found in get, then return NULL
* Multi-threaded long running test to check for memory leaks. 
* Run valgrind on the code

Done.
====
* Divided into regions 
* Locks on individual regions.
* Reads without locking.
* Writes lock individual regions.
* If key hashes to same location, then add to linked list
*/
