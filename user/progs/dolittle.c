/** @file donothing.c
 *  @brief A simple test that speaks and returns.
 *  @author Enrique Naudon (esn)
 **/

#include <syscall.h>

#define LEN 4

int main()
{
  char buf[LEN] = {'A','B','C','\0'};
  print(LEN, buf);
  return 0;
}
