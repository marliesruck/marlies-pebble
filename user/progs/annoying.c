/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */


#define DELAY (16*1024)

static volatile int no_opt = 0;

void foo() {  ++no_opt;  }
int bar() {  return 1;  }

void slow()
{
  int i;
  for (i = 0; i < DELAY; i += bar()) foo();
}

int main()
{
  unsigned int ticks;

  while(1) {
    ticks = get_ticks();
    lprintf("tick count = %d", ticks);
    slow();
  }
}
