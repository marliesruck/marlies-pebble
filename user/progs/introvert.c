/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */
#include <spin.h>

spin_s spin = {0,0};
int main() 
{
  int ch;
  while(1){
    spin_lock(&spin);
    lprintf("Who am I? Press 'r' to find out!");
    while((ch = getchar()) != 'r');
    lprintf("I'm introverted...my tid is %d",gettid());
    spin_unlock(&spin);
  }
}
