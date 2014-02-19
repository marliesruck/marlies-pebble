/** @file frame_alloc.c
 *
 *  @brief Implements our frame allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* Frame allocator includes */
#include <frame_alloc.h>

/* Pebbles includes */
#include <common_kern.h>
#include <x86/page.h>

/* Libc includes */
#include <stddef.h>


unsigned int frame_index = 0;
char (*frames)[PAGE_SIZE] = (char (*)[PAGE_SIZE]) USER_MEM_START;

void *alloc_frame(void)
{
  void *base;
  base = (void *)&frames[frame_index];
  ++frame_index;
  return base;
}

void free_frame(void *frame)
{
  return;
}

