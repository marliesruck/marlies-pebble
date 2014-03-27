/* @file rogue.c
 *
 * Test if there are any race conditions between parent and child vanishing, as
 * well as our code for init reaping children.
 *
 */
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main(){
  while(1){
    if(fork()){
      lprintf("parent vanishing");
      vanish();
    }
  }
}
