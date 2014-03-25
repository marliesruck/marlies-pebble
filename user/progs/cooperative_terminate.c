/** @file user/progs/cooperative_terminate.c
 *  @author mpa
 *  @brief Tests that deschedule() doesn't return without a corresponding
 *         make_runnable() call.
 *  @public yes
 *  @for p3
 *  @covers deschedule
 *  @status done
 */

#include <simics.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <report.h>
#include "../../410user/inc/410_tests.h"

#define LIM 16 
#define DELAY (16*1024)

DEF_TEST_NAME("cooperative:");

static volatile int no_opt = 0;
static int count = 0;

void foo() {  ++no_opt;  }
int bar() {  return 1;  }

void slow()
{
  int i;
  for (i = 0; i < DELAY; i += bar()) foo();
}

int main()
{
  int parent, child;
  int reject;

  report_start(START_CMPLT);

  reject = 0;
  parent = gettid();
  child = fork();

  /* Parent */
  if (child) {
    lprintf("parent running...");

    while (count < LIM) {
      lprintf("p: descheduling(&reject)");
      deschedule(&reject);
      lprintf("p: awake!");

      lprintf("p: make_runnable(child)");
      make_runnable(child);

      slow();
      count++;
    }
  }

  /* Child */
  else {
    lprintf("child running...");

    while (count < LIM) {
      lprintf("c: make_runnable(parent)");
      make_runnable(parent);

      lprintf("c: descheduling(&reject)");
      deschedule(&reject);
      lprintf("c: awake!");

      slow();
      count++;
    }
  }

  report_end(END_SUCCESS);

  vanish();
}
