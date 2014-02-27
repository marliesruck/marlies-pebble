/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */


int main() {
//  while(1){
    lprintf("In Introvert...");
 // }
  while(1){
    if(getchar() > 0)
      lprintf("introvert");
  }

    while(1) continue;
    /*
	int pid;

	pid = gettid();
	lprintf("My pid is %d and I'm a bit introverted...", pid);
	
  int i = 0;
  while (1){
    if(i%1000 == 0)
      lprintf("!");
    i++;
  }
  */
}
