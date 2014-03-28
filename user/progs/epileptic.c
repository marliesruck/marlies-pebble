/** @file 410user/progs/sleep_test1.c
 *  @author zra
 *  @brief Sleeps for amount of time given as an argument.
 *  @public yes
 *  @for p3
 *  @covers sleep
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for sleep, getpid */
#include <stdio.h>
#include <simics.h>   /* for lprintf */
#include <stdlib.h>   /* for atoi, exit */
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("sleep_test1:");

int main(int argc, char *argv[])
{
  int before, after;
  int i;
  int tid;
  report_start(START_CMPLT);
  report_misc("before sleeping");

  fork();
  fork();

  tid = gettid();

  for (i = 0; i < 30; i += 5) {
    lprintf("%d sleeping for %d ticks", i, tid);
    before = get_ticks();
	  sleep(i);
    after = get_ticks();
    lprintf("%d is awake!", tid);
    if (before + i > after) {
      lprintf("%d under slept :(", tid);
      report_end(END_FAIL);
    }
  }

  report_misc("after sleeping");
  report_end(END_SUCCESS);
  exit( 42 );
}
