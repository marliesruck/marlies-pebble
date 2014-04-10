/** @file 410user/progs/deschedule_hang.c
 *  @author mpa
 *  @brief Tests that deschedule() doesn't return without a corresponding
 *         make_runnable() call.
 *  @public yes
 *  @for p3
 *  @covers deschedule
 *  @status done
 */

#include <assert.h>
#include <simics.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <report.h>
#include <thread.h>


#define DELAY (16*1024)

int parent, child;
int p_reject, c_reject;


static volatile int no_opt = 0;

void foo() {  ++no_opt;  }
int bar() {  return 1;  }

void slow()
{
  int i;
  for (i = 0; i < DELAY; i += bar()) foo();
}

void *child_fn(void *args)
{
  assert(parent);

  lprintf("child running...");

  while (1) {
    p_reject = 1;
    lprintf("c: p_reject = %d", p_reject);
    lprintf("c: make_runnable(parent)");
    make_runnable(parent);

    lprintf("c: descheduling(&c_reject=%d)", c_reject);
    c_reject = 0;
    while (!c_reject)
      lprintf("c: deschedule(*c_reject=%d) = %d", c_reject, deschedule(&c_reject));
    lprintf("c: awake!");

    slow();
  }

  return NULL;
}

int main()
{
  thr_init(PAGE_SIZE);

  parent = gettid();
  child = thr_create(child_fn, NULL);
  assert(child);

  lprintf("parent running...");

  while (1) {
    lprintf("p: descheduling(&p_reject=%d)", p_reject);
    p_reject = 0;
    while (!p_reject)
      lprintf("p: deschedule(*p_reject=%d) = %d", p_reject, deschedule(&p_reject));
    lprintf("p: awake!");

    c_reject = 1;
    lprintf("p: c_reject = %d", c_reject);
    lprintf("p: make_runnable(child)");
    make_runnable(child);

    slow();
  }

  exit(1);
}
