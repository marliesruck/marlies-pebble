/**
 * @file cvar.c
 * @brief Basic cvar functionality
 *
 *
 * @author Joey Echeverria (jge)
 * @bug No known bugs.
 */

#include <simics.h>

#include <thread.h>
#include <syscall.h>
#include <cond.h>
#include <mutex.h>
#include <stdio.h>
#include "410_tests.h"
DEF_TEST_NAME("mutex_destroy_test:");

#define STACK_SIZE 4096
#define NUM_GUARDS 12
#define NUM_MINIONS 10

cond_t cvar;
mutex_t mutex;

int ktids[NUM_MINIONS];

void *print_fn(void *args)
{
  int id;
  
  id = (int) args;

//  for (i = 0; i < NUM_GUARDS; ++i) {
    mutex_lock(&mutex);
    lprintf("Thread %d locked mutex and entering cond wait", id);
    cond_wait(&cvar,&mutex);
    lprintf("Thread %d received cond signal\n",id);
    if(id == 0)
	    ktids[id] = 25;
    else
    	ktids[id] = id;
    mutex_unlock(&mutex);
//    cond_signal(&cvar);
//  }

//  lprintf("Thread %d looping", id);
  while (1) yield(-1);

  return NULL;
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
	int error, i;
  REPORT_LOCAL_INIT;

  REPORT_START_ABORT;
	if((error = thr_init(STACK_SIZE)) < 0) {
		REPORT_ERR("thr_init() returned error %d", error);
		REPORT_END_FAIL;
    return -10;
	}

	if((error = cond_init(&cvar)) < 0) {
		REPORT_ERR("cvar_init() returned error %d", error);
		REPORT_END_FAIL;
		return -20;
	}
	mutex_init(&mutex);
  lprintf("root thread spawns peers\n");
  for (i = 0; i < NUM_MINIONS; ++i)
    thr_create(print_fn, (void *)i);

  cond_broadcast(&cvar);

  int done = 0;

  while (!done) {
    int slot,nregistered;
    for (nregistered = 0, slot = 0; slot < NUM_MINIONS; ++slot)
      if (ktids[slot] != 0)
        ++nregistered;
    if (nregistered == NUM_MINIONS)
      done = 1;
    else{
      lprintf("nregistered: %d\n",nregistered);
      sleep(1);
      cond_broadcast(&cvar);
    }
  }

  lprintf("destroy cvar\n");
  cond_destroy(&cvar);

  lprintf("main looping");
  while (1) yield(-1);

	REPORT_END_FAIL;
	return 1;
}

