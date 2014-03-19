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

DEF_TEST_NAME("mutex:");

#define STACK_SIZE 4096
#define NUM_GUARDS 10
#define NUM_MINIONS 5

mutex_t mutex[NUM_GUARDS];

void *print_fn(void *args)
{
  int id, i;
  
  id = (int) args;

  for (i = 0; i < NUM_GUARDS; ++i) {
    lprintf("Thread %d locking %d!", id, i);
    mutex_lock(&mutex[i]);
    lprintf("Thread %d acquired mutex %d!", id, i);
    sleep(1);
    lprintf("Thread %d releasing mutex %d!", id, i);
    mutex_unlock(&mutex[i]);
  }

  lprintf("Thread %d exiting!", id);
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
  int tids[NUM_MINIONS];
  REPORT_LOCAL_INIT;

  REPORT_START_CMPLT;
	if((error = thr_init(STACK_SIZE)) < 0) {
		REPORT_ERR("thr_init() returned error %d", error);
		REPORT_END_FAIL;
    return -10;
	}

  for (i = 0; i < NUM_GUARDS; ++i) {
	  if((error = mutex_init(&mutex[i])) < 0) {
	  	REPORT_ERR("mutex_init() returned error %d", error);
	  	REPORT_END_FAIL;
	  	return -20;
	  }
  }

  mutex_lock(&mutex[0]);
  for (i = 0; i < NUM_MINIONS; ++i)
    tids[i] = thr_create(print_fn, (void *)i);
  mutex_unlock(&mutex[0]);

  lprintf("main reaping");
  for (i = 0; i < NUM_MINIONS; ++i) {
    thr_join(tids[i], (void **)&status);
    lprintf("%d: main joined %d with %d", i, tids[i], status);
  }

  REPORT_END_SUCCESS; 
	return 0;
}

