/** @file schizo.c
 *
 *  @brief Exec's a new process.
 *
 *  @author Enrique Naudon (esn)
 **/
#include <syscall.h>
#include <simics.h>
#include <stddef.h>

#define NUM_ARGS 5

int main()
{
  char *bin = "introspective";
  char *argvec[NUM_ARGS] = { bin, "-v", "-O2", "-ggdb", NULL };

  lprintf("exec'ing...");
  exec(bin, argvec);
	
  while (1) continue;
  return 0;
}

