#include <check.h>
#include "../src/rwlock.h"
#include <errno.h>

START_TEST(test_init_rwlock) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *lock_ptr = &lock;
  ck_assert_ptr_ne(lock_ptr, NULL);
  ck_assert_int_eq(lock_ptr->r_count, 0);
  ck_assert_int_eq(lock_ptr->w_count, 0);
  ck_assert_uint_eq(lock_ptr->valid, VALID_RW_LOCK);

  destroy_lock(&lock);
}

START_TEST(test_destroy_lock) {
  rwlock_t lock;

  int status = init_lock(&lock);
  rwlock_t *lock_ptr = &lock;

  status = destroy_lock(lock_ptr);
  ck_assert_int_eq(status, 0);
  ck_assert_uint_eq(lock_ptr->valid, 0);
}

START_TEST(test_read_lock_return_invalid_when_lock_not_initialized) {
  rwlock_t lock;
  
  int status = readlock(&lock);
  ck_assert_uint_eq(status, EINVAL);
}

START_TEST(test_read_lock_increments_readers) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *lock_ptr = &lock;

  status = readlock(lock_ptr);
  ck_assert_int_eq(status, 0);

  ck_assert_int_eq(lock_ptr->r_count, 1);
}

START_TEST(test_write_lock_increments_writers) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *lock_ptr = &lock;

  status = writelock(lock_ptr);
  ck_assert_int_eq(status, 0);

  ck_assert_int_eq(lock_ptr->w_count, 1);
}

START_TEST(test_write_unlock_decrements_writers) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *lock_ptr = &lock;

  status = writelock(lock_ptr);
  ck_assert_int_eq(status, 0);
  ck_assert_int_eq(lock_ptr->w_count, 1);

  status = writeunlock(lock_ptr);
  ck_assert_int_eq(status, 0);
  ck_assert_int_eq(lock_ptr->w_count, 0);
}

START_TEST(test_read_unlock_decrements_readers) {
  rwlock_t lock;

  int status = init_lock(&lock);
  ck_assert_int_eq(status, 0);
  rwlock_t *lock_ptr = &lock;

  status = readlock(lock_ptr);
  ck_assert_int_eq(status, 0);
  ck_assert_int_eq(lock_ptr->r_count, 1);

  status = readunlock(lock_ptr);
  ck_assert_int_eq(status, 0);
  ck_assert_int_eq(lock_ptr->r_count, 0);
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
  tcase_add_test(tc_core, test_read_lock_return_invalid_when_lock_not_initialized);
  tcase_add_test(tc_core, test_read_lock_increments_readers);
  tcase_add_test(tc_core, test_write_lock_increments_writers);
  tcase_add_test(tc_core, test_read_unlock_decrements_readers);
  tcase_add_test(tc_core, test_write_unlock_decrements_writers);

  suite_add_tcase(s, tc_core);
  return s;
}

#endif
