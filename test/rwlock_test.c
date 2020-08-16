#include <check.h>
#include "../src/rwlock.h"

START_TEST(test_init_rwlock) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *init_lock = &lock;
  ck_assert_ptr_ne(init_lock, NULL);
  ck_assert_int_eq(init_lock->r_count, 0);
  ck_assert_int_eq(init_lock->w_count, 0);
  ck_assert_uint_eq(init_lock->valid, VALID_RW_LOCK);

  destroy_lock(&lock);
}

START_TEST(test_destroy_lock) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *init_lock = &lock;

  status = destroy_lock(&lock);
  ck_assert_int_eq(status, 0);
  ck_assert_uint_eq(init_lock->valid, 0);
}

#ifndef __RWLOCK_SUITE_
#define __RWLOCK_SUITE_

Suite* rwlock_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("rwlock suite");
  tc_core = tcase_create("Core tests");

  tcase_add_test(tc_core, test_init_rwlock);
  tcase_add_test(tc_core, test_destroy_lock);

  suite_add_tcase(s, tc_core);
  return s;
}

#endif
