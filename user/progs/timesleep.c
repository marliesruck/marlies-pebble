/** @file timesleep.c
 *  @brief A sleepy test.
 *
 *  The time before sleep plus the sleep time should be roughly equal to
 *  the time after we wake.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <simics.h>

#include <syscall.h>
#include <stddef.h>

#define SLEEP_TICKS 100

int main()
{
  int before, after;

  before = get_ticks();
  sleep(SLEEP_TICKS);
  after = get_ticks();

  lprintf("before=%d", before);
  lprintf("sleep=%d", SLEEP_TICKS);
  lprintf("after=%d", after);

  return 0;
}

