/** @file user/prog/rogue.c
 *  @file Tests init reaping children.
 *  @for p3
 *  @covers fork set_status vanish
 *  @status done
 */
#include <simics.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int pid = 0;
    int count = 0;

	while(count < 1000) {
    if((pid = fork())){
      lprintf("parent: %d exiting", gettid());
      exit(0);
    }
    count++;
	}

	exit(42);
}
