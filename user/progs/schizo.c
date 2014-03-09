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

int main(int argc, char **argv)
{
  char *bin = "schizo";
  char *argvec[NUM_ARGS] = { bin, "-v", "-O2", "-ggdb", NULL };
  int i;

  lprintf("TID = %d", gettid());
  for (i = 0; i < argc; ++i)
    lprintf("  argv[%d] = %s", i, argv[i]);

  lprintf("exec'ing...");
  exec(bin, argvec);
	
  while (1) continue;
  return 0;
}

