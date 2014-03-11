/** @file zfod.c
 *  @brief Tests fork, exec, and context-switching with zfod
 *  @covers fork gettid exec zfod
 */

#include <syscall.h>
#include <simics.h>

char *program = "peon";
char *args[2];

/* Make this big enough to span multiple pages */
int test[4096];

int main() {

  if(fork() == 0){
    lprintf("in child modifying test...");
    test[4095] = 798;
    lprintf("in child test[4095] should be 789 and is %d and test[2048] should "
            "be 0 and is %d", test[4095], test[2048]);
    if (fork() == 0)
      lprintf("in grand child test[4095] should be 789 and is %d, test[2048] should "
            "be 0 and is %d", test[4095], test[2048]);
  }
  lprintf("in parent");

  while(1);
}
