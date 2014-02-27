/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

int main() {
// while(1){
  lprintf("In Introspective...");
// }
  while(1){
    if(getchar() > 0)
      lprintf("introspective");
  }
  /*
	int pid;

	pid = gettid();
	lprintf("Q: What happened to thread 9?  A: %d.", pid);

  int i = 0;
  while (1){
    if(i%1000 == 0)
      lprintf("*");
    i++;
  }
  */
}
