/** @file pagefault.c
 *  @brief A simple test initializes the thread library and crashes.
 *  @author Enrique Naudon (esn)
 **/

#include <syscall.h>
#include <thread.h>


int main()
{
  int zero = 0;

  thr_init(PAGE_SIZE);

  return 1/zero;
}

