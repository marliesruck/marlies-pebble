/* @file vanish_test.c
 *
 */
#include <syscall.h>
#include <simics.h>

int main(){
  int tid = fork();
  if(tid){
    lprintf("Parent about to vanish");
    vanish();
  }
  else{
    while(1)
      lprintf("In child!");
  }
}
