/** @file donothing.c
 *  @brief A simple test that asserts and fails.
 *  @author Enrique Naudon (esn)
 **/

#include <assert.h>


int main()
{
  assert(-1 == 3 || 2 == 123);

  return -1;
}
