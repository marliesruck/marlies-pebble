/** @file pagefault.c
 *  @brief A simple test that page faults.
 *  @author Enrique Naudon (esn)
 **/

#include <stddef.h>

int main()
{
  int *null;
  null = NULL;
  *null = 1;

  return -1;
}

