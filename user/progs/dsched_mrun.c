/** @file dsched_mrun.c
 *  @brief A deschedling, runnable-making test.
 *
 *  We expect the child's printout before the parents every time.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <simics.h>

#include <syscall.h>
#include <stddef.h>

int main()
{
  int id;

  id = fork();
  if (id) {
    make_runnable(id);
    yield(id);
    lprintf("parent: kids first!");
  }
  else {
    deschedule(NULL);
    lprintf("child: MEMEMEMEMEME");
  }

  return 0;
}

