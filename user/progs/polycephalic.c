/** @file mutex.c
 *
 *  @brief A simpler (thread group-free) version of agility_drill.
 *
 *  @author Enrique Naudon (esn)
 */

#include <simics.h>

#include "410_tests.h"

#include <thread.h>
#include <mutex.h>
#include <syscall.h>
#include <stdio.h>

DEF_TEST_NAME("polycephalic:");

#define STACK_SIZE 4096
#define NUM_HEADS 5

void *print_fn(void *args)
{
  int id;
  id = (int) args;
  lprintf("Thread %d alive!", id);
  return (void *)id;
}

/**
 * @brief
 *
 * @param argc The number of arguments
 * @param argv The argument array
 * @return 1 on success, < 0 on error.
 */
int main(int argc, char *argv[])
{
	int error, i, status;
  int tids[NUM_HEADS];
  REPORT_LOCAL_INIT;

  REPORT_START_CMPLT;
	if((error = thr_init(STACK_SIZE)) < 0) {
		REPORT_ERR("thr_init() returned error %d", error);
		REPORT_END_FAIL;
    return -10;
	}

  lprintf("main spawning");
  for (i = 0; i < NUM_HEADS; ++i) {
    tids[i] = thr_create(print_fn, (void *)i);
  }

  lprintf("main reaping");
  for (i = 0; i < NUM_HEADS; ++i) {
    thr_join(tids[i], (void **)&status);
    lprintf("%d: main joined %d with %d", i, tids[i], status);
  }

  REPORT_END_SUCCESS; 
	return 0;
}

