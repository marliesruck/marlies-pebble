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

#define INT32_MAX 2147483647 

int parent, child;
int p_reject, c_reject;

void *child_fn(void *args)
{
  lprintf("c: %d", gettid());
  return NULL;
}

int main()
{
  thr_init(PAGE_SIZE);

  int start_tid = gettid();
  lprintf("root thread tid: %d", start_tid);

  for(; start_tid < INT32_MAX; ++start_tid){
    child = thr_create(child_fn, NULL);
    assert(child);
  }

  lprintf("***** start tid is INT32_MAX: %d ******", INT32_MAX);
  MAGIC_BREAK;

  while(1){
    child = thr_create(child_fn, NULL);
    assert(child);
  }


  exit(1);
}
