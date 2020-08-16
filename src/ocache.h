#ifndef __OCACHE_H_
#define __OCACHE_H_

#include <time.h>

typedef struct value_tag {
  void *data;
  time_t ttl;
} value_t;

extern void init();
extern void put(int key, value_t* value);
extern value_t* get(int key);
extern void destroy();

#endif
