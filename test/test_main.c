#include <check.h>
#include <sys/types.h>
#include <stdlib.h>

extern Suite* ocache_suite();
extern Suite* rwlock_suite();

int main(void) {
  int no_of_failed = 0;
  Suite* s;
  SRunner *runner;

  s = ocache_suite();
  runner = srunner_create(s);

  srunner_add_suite(runner, rwlock_suite());
  
  srunner_run_all(runner, CK_NORMAL);
  no_of_failed = srunner_ntests_failed(runner);

  srunner_free(runner);
  return (no_of_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
