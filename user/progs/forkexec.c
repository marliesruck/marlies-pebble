/** @file forkexec.c
 *  @brief A forking and execing test.
 *
 *  The expected behavior of this test is to print ABC, as dolittle does,
 *  but return 123, as donothing.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall.h>
#include <stddef.h>

#define LEN 1

int main()
{
  int id;
  char *args[LEN] = {NULL};

  id = fork();
  if (!id) exec("dolittle", args);

  return 123;
}

