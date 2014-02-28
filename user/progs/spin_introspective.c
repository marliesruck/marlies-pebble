/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */
#include <spin.h>

spin_s spin;

int main() {
  spin_init(&spin);
  lprintf("In Introspective...");
  /*
  while(1){
    if(getchar() > 0)
      lprintf("introspective");
  }
  */
  while(1){
    spin_lock(&spin);
    lprintf("Introspective");
    spin_unlock(&spin);
  }
}
