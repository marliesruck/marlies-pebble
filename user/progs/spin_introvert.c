/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */
#include <spin.h>

spin_s spin;

int main() {
  spin_init(&spin);
  lprintf("In introvert...");
  /*
  while(1){
    if(getchar() > 0)
      lprintf("introspective");
  }
  */
  while(1){
    spin_lock(&spin);
    lprintf("introvert");
    spin_unlock(&spin);
  }
}
