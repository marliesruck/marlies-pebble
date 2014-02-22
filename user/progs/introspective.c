/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */


int main() {
	int pid;

	pid = gettid();
	lprintf("Who am I?  I am %d.", pid);
	
  while (1) continue;
}
