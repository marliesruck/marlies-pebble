/** @file vanishwait.c
 *  @brief A forking, vanishing and waiting test.
 *
 *  This test should return 666 from it's child, NOT -1.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall.h>
#include <stddef.h>

int main()
{
  int id, status;

  id = fork();
  if (!id) task_vanish(666);

  status = -1;
  wait(&status);

  return status;
}

