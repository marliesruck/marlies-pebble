/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */
#include <spin.h>
#include "my_spin.h"

extern spin_s spin;
int main() {
  int ch;
  lprintf("In introvert");
  while(1){
    spin_lock(&spin);
    lprintf("introvert: press 'r' to release the lock!");
    while((ch = getchar()) != 'r');
    spin_unlock(&spin);
    lprintf("introvert: press 't' to take the lock!");
    while((ch = getchar()) != 't');
  }
}
