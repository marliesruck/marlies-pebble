/** @file schizo.c
 **/

#include <syscall.h>
#include <simics.h>
#include <stddef.h>


int main()
{
  char *argvec[4] = { NULL };

  argvec[0] = "arg 1";
  argvec[1] = "arg 2";
  argvec[2] = "arg 3";

  lprintf("exec'ing...");
  exec("introspective", argvec);
	
  while (1) continue;
  return 0;
}

