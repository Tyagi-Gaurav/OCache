#ifndef __OCACHE_H
#define __OCACHE_H

#include <pthread.h>
#include <stdio.h>

typedef struct value_tag {
  void *data;
  time_t ttl;
} value_t;

void init();
void put(int key, value_t* value);
void put_if_absent(int key, value_t* value);
value_t* get(int key);
void destroy();

#endif
