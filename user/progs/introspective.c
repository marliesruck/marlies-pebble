/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */
#include <spin.h>
//#include "my_spin.h"

//extern spin_s spin;
//spin_s spin = {0,0};
int main(){

spin_s spin = {0,0};
  lprintf("In introspective");
  int ch;
  while(1){
    spin_lock(&spin);
    lprintf("introspective: press 'r' to release the lock!");
    while((ch = getchar()) != 'r');
    spin_unlock(&spin);
    lprintf("introspective: press 't' to take the lock!");
    while((ch = getchar()) != 't');
  }

}
