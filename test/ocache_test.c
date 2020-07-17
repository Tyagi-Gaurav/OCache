#include <check.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../src/ocache.h"
#include <stdio.h>

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
  value_t* ac_val = (value_t*)actual;
  ck_assert_ptr_eq(ac_val->data, (int *)24);
  ck_assert_int_eq(ac_val->ttl, exp_ttl);

  actual = get(183);
  ck_assert_ptr_ne(actual, NULL);
  ac_val = (value_t*)actual;
  ck_assert_ptr_eq(ac_val->data, (int *)26);
  ck_assert_int_eq(ac_val->ttl, exp_ttl);

  actual = get(199);
  ck_assert_ptr_ne(actual, NULL);
  ac_val = (value_t*)actual;
  ck_assert_ptr_eq(ac_val->data, (int *)28);
  ck_assert_int_eq(ac_val->ttl, exp_ttl);

  destroy();
}

Suite* ocache_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("cache suite");
  tc_core = tcase_create("Core tests");

  tcase_add_test(tc_core, test_cache_single_write_and_read);

  suite_add_tcase(s, tc_core);
  return s;
}

int main(void) {
  int no_of_failed = 0;
  Suite* s;
  SRunner *runner;

  s = ocache_suite();
  runner = srunner_create(s);

  srunner_run_all(runner, CK_NORMAL);
  no_of_failed = srunner_ntests_failed(runner);

  srunner_free(runner);
  return (no_of_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
