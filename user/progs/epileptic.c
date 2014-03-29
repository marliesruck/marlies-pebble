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

DEF_TEST_NAME("epileptic:");

int main(int argc, char *argv[])
{
  int before, after, slept;
  int i;
  int tid;
  report_start(START_CMPLT);

  fork();
  fork();

  tid = gettid();
  
  /* To mix it a little */
  sleep(tid);

  for (i = 0; i < 30; i += 5) {
    lprintf("%d: sleeping for %d ticks", tid, i);
    before = get_ticks();
	  sleep(i);
    after = get_ticks();
    slept = after - before;
    lprintf("%d: slept for %d... that's enough!", tid, slept);
    if (slept < i) {
      lprintf("%d: slept for %d... we under slept :(", tid, slept);
      report_end(END_FAIL);
    }
  }

  report_end(END_SUCCESS);
  exit( 42 );
}
