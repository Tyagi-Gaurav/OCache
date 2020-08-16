#include <check.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../src/ocache.h"
#include <stdio.h>

START_TEST(test_cache_single_read) {
  init();
  value_t *actual = get(254);
  ck_assert_ptr_eq(actual, NULL);
  destroy();
}

START_TEST(test_cache_single_write_and_read) {
  init();
  time_t exp_ttl = time(NULL);
  value_t *val = malloc(sizeof(val));
  val->data = (int *)24;
  val->ttl = exp_ttl;
  put(254, val);
  
  val = malloc(sizeof(val));
  val->data = (int *)26;
  val->ttl = exp_ttl;
  put(183, val);

  val = malloc(sizeof(val));
  val->data = (int *)28;
  val->ttl = exp_ttl;
  put(199, val);

  value_t *actual = get(254);
  ck_assert_ptr_ne(actual, NULL);
  ck_assert_ptr_eq(actual->data, (int *)24);
  ck_assert_int_eq(actual->ttl, exp_ttl);

  actual = get(183);
  ck_assert_ptr_ne(actual, NULL);
  ck_assert_ptr_eq(actual->data, (int *)26);
  ck_assert_int_eq(actual->ttl, exp_ttl);

  actual = get(199);
  ck_assert_ptr_ne(actual, NULL);
  ck_assert_ptr_eq(actual->data, (int *)28);
  ck_assert_int_eq(actual->ttl, exp_ttl);
  destroy();
}

START_TEST(test_cache_multiple_write_increase_cache_size) {
  init();
  time_t exp_ttl = time(NULL);
  value_t *val = malloc(sizeof(val));
  val->data = (int *)24;
  val->ttl = exp_ttl;
  put(254, val);
  put(183, val);
  put(199, val);
  put(278, val);
  put(173, val);

  destroy();
}

#ifndef __OCACHE_SUITE_
#define __OCACHE_SUITE_

Suite* ocache_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("cache suite");
  tc_core = tcase_create("Core tests");

  tcase_add_test(tc_core, test_cache_multiple_write_increase_cache_size);
  tcase_add_test(tc_core, test_cache_single_write_and_read);
  tcase_add_test(tc_core, test_cache_single_read);
  
  suite_add_tcase(s, tc_core);
  return s;
}
#endif


