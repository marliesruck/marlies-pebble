/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */


int main() {
	int pid;

	pid = gettid();
	lprintf("Q: What happened to thread 9?  A: %d.", pid);
	
  while (1) continue;
}
