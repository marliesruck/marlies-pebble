/** @file 410user/progs/peon.c
 *  @author mpa
 *  @brief Tests exec() by launching merchant.
 *  @public yes
 *  @for p3
 *  @covers exec
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <stddef.h>
#include <simics.h>

int main() {

  int tid = gettid();

  char buf[16];
  char * prog = "merchant_terminate";
  char * args[5] = { "merchant", "13", "foo bar", NULL, NULL };

  args[3] = buf;
  sprintf(buf, "%d", tid);

  lprintf("promoting peon #%d to a merchant", tid);

  if (exec(prog, args) < 0)
    lprintf("ABORTING: exec returned error");
  else
    lprintf("ABORTING: exec returned success");

  return 0;
}
