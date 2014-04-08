/** @file user/progs/loquacious.
 *
 *  Fork cho_variant until it fails.
 *
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <simics.h>

int main()
{
  char *args[5];
  args[0] = "print_basic";
  args[1] = NULL;

  while(1){
    if(!fork()) exec(args[0], args);
    while(wait(NULL) >= 0);
  }

  return 0;

}
