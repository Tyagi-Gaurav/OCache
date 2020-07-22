
#include "ocache.h"
#include <math.h>
#include <signal.h>
#include <execinfo.h>

#define __INIT_SIZE 4
#define __OFFSET_BASIS 0xcbf29ce484222325
#define __PRIME 0x100000001b3
#define __REGIONS 32

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
  pthread_mutex_t mutex_resize;
  pthread_mutex_t mutex;
  cache_entry_t *cache;
  int c_size;
  int t_size;
} region_t;

region_t region[__REGIONS];
int status;

void handler(int sig) {
  void *array[10];
  size_t size;

  //Get all entries on the stack.
  size = backtrace(array, 10);

  //print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void init() {
  #ifdef __DEBUG
  printf("Initializing Cache.\n");
  #endif
  signal(SIGTRAP, handler);
  signal(SIGSEGV, handler);
  for(int i=0;i < __REGIONS;++i) {
    status = pthread_mutex_init(&region[i].mutex, NULL);
    __ERR_REPO(status, "Mutex initialization");

    status = pthread_mutex_init(&region[i].mutex_resize, NULL);
    __ERR_REPO(status, "Mutex initialization");

    region[i].cache = malloc(sizeof(cache_entry_t) * __INIT_SIZE);
    memset(region[i].cache, 0, sizeof(cache_entry_t) * __INIT_SIZE);
    
    region[i].c_size = 0;
    region[i].t_size = __INIT_SIZE;
  }
#ifdef __DEBUG
  printf("Cache Initialized.\n");
#endif
}

void destroy() {}

int find_location(uint64_t hash, int size) {
  uint32_t l = (uint32_t) hash;
  uint32_t h = hash >> 32;
  uint32_t a = l&h;
  int loc = a % size;
#ifdef __DEBUG
  printf("Using l: %lu, h:%lu, l&h: %lu, size: %d, loc: %d\n", l, h, a, size, loc);
#endif
  return (l&h) % size;
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

void add_to_cache(cache_entry_t *cache, int key, void *val) {
  //Adding key to node for the first time.
    if (cache->node == NULL) {
      node_t *new_nd = malloc(sizeof(node_t));
      new_nd->next = NULL;
      new_nd->key = key;
      new_nd->value = val;
      cache->node = new_nd;
      cache->end = cache->node;
#ifdef __DEBUG
      printf("Added for key: %d, old val: %ld, new val: %ld\n", key, val, cache->node->value);
#endif
    } else if ((cache->node)->key == key) {
      //Same element exists at the location.
      (cache->node)->value = val;
    } else {
      //There's already a value at the location. Add to linked list.
#ifdef __TRACE
      printf("Place already taken by key: %lu\n", cache->node->key);
#endif
      node_t *new_nd = malloc(sizeof(node_t));
      new_nd->next = NULL;
      new_nd->key = key;
      new_nd->value = val;
      (cache->end)->next = new_nd;
      cache->end = new_nd;
    }
}

void put(int key, value_t* val) {
  uint64_t hash = calculateHash(key);
  int r = hash % __REGIONS; //region
  region_t reg = region[r];
  
  status = pthread_mutex_lock(&reg.mutex);
  __ERR_REPO(status, "Mutex lock");

  int l = find_location(hash, region[r].t_size);
  if (region[r].c_size+1 <= region[r].t_size) {
#ifdef __DEBUG
    printf("Thread %d: Adding to region %d: %d with TTL %ld\n", pthread_self(), r, key, ((value_t*)val)->ttl);
#endif
    add_to_cache(&region[r].cache[l], key, val);
    region[r].c_size++;
  } else {
#ifdef __TRACE
    printf("Region %d full. Cannot add key: %d\n", r, key);
#endif

    status = pthread_mutex_lock(&region[r].mutex_resize);
    __ERR_REPO(status, "Resize mutex lock");
    
    //Create new cache that is double in size
    int n_size = region[r].t_size * 2;
    cache_entry_t *new_cache = malloc(sizeof(cache_entry_t) * n_size);
    memset(new_cache, 0, sizeof(cache_entry_t) * n_size);

#ifdef __DEBUG
    printf("Starting to resize region %d to %d.\n", r, n_size);
#endif
    region[r].t_size = n_size;
    
     //Rehash
    for (int i=0; i < region[r].c_size;++i) {
      node_t *node = region[r].cache[i].node;

      while (node != NULL) {
	int k = node->key;
	void *v = node->value;
	uint64_t nh = calculateHash(k);
	int nl = find_location(nh, n_size);
#ifdef __DEBUG
	printf("Thread %d: Re-adding to region %d:%d %d with old TTL %ld, new TTL %ld\n", pthread_self(), r, nl, k, node->value, v);
#endif
	node->key = 0; node->value = NULL;
	add_to_cache(&new_cache[nl], k, v);
#ifdef __TRACE
	printf("Thread %d: Re-added to region %d:%d %d with TTL %ld\n", pthread_self(), r, nl, k, v);
#endif
	node = node->next;
#ifdef __TRACE
	printf("Thread %d: While re-adding. Got next node.\n");
#endif	
      }//End-while
    }//End-for
    
    //Add the new element to cache
    hash = calculateHash(key);
    l = find_location(hash, n_size);
#ifdef __DEBUG
    printf("Thread %d: Adding original to region %d: %d with TTL %ld\n", pthread_self(), r, key, ((value_t*)val)->ttl);
#endif    
    add_to_cache(&new_cache[l], key, val);

    //Update indices and free old cache
    cache_entry_t *old_cache = region[r].cache;
    region[r].cache = new_cache;
    region[r].c_size++;
    free(old_cache);
    
    status = pthread_mutex_unlock(&region[r].mutex_resize);
    __ERR_REPO(status, "Resize mutex unlock");
  }//End-if

#ifdef __TRACE
  printf("Thread %d: Added key %d to region:location: %d:%d and size is now: %d\n", pthread_self(), key,r, l, region[r].c_size);
#endif
  
  status = pthread_mutex_unlock(&reg.mutex);
  __ERR_REPO(status, "Mutex unlock");
}

void put_if_absent(int key, value_t* val) {}

value_t *get(int key) {
  value_t *result = NULL;
  uint64_t hash = calculateHash(key);
  int r = hash % __REGIONS; //region
  
  status = pthread_mutex_lock(&region[r].mutex_resize);
  __ERR_REPO(status, "Resize mutex lock");

#ifdef __DEBUG
  printf("Looking for key %d in region %d\n", key, r);
#endif
  
  int l = find_location(hash, region[r].t_size);

  node_t *n = region[r].cache[l].node;
  while (n != NULL && n->key != key) n = n->next;

  if (n != NULL)
    result = n->value;

  status = pthread_mutex_unlock(&region[r].mutex_resize);
  __ERR_REPO(status, "Resize mutex unlock");
  
  return result;
}

/*
TODO
====
* Anything with cache lines?

Done.
====
* Dynamically increase the cache size per region.
* Install Trace/BPT and SIGNAL handlers to exit gracefully.
* Divided into regions 
* Locks on individual regions.
* Reads without locking.
* Writes lock individual regions.
* If key hashes to same location, then add to linked list
* During get search for all the keys in the linked list.
* If element not found in get, then return NULL
* Run valgrind on the code
   valgrind -v --leak-check=full ./ocache_multi
* Multi-threaded long running test to check for memory leaks. 
*/
