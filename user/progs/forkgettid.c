/** @file forkgettid.c
 *  @brief A forking and tid-getting test.
 *
 *  We expect the child's TID to be the same according to fork(...) and
 *  gettid(...).
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
  if (id) lprintf("parent: fork() = %d", id);
  else lprintf("child: gettid() = %d", gettid());

  return 0;
}

