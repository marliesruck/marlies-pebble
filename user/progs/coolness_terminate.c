/** @file 410user/progs/coolness.c
 *  @author de0u
 *  @brief Tests fork, exec, and context-switching.
 *  @public yes
 *  @for p3
 *  @covers fork gettid exec
 *  @status done
 */

#include <syscall.h>
#include <simics.h>
#include <report.h>
#include "../../410user/inc/410_tests.h"

char *program = "peon_terminate";
char *args[2];

DEF_TEST_NAME("coolness:");

int main() {

  report_start(START_CMPLT);

  fork();
  args[0] = program;
  fork();

  exec(program, args);

  vanish();
}
