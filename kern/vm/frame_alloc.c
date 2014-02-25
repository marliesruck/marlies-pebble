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

#include <simics.h>


typedef char frame_t[PAGE_SIZE];
const frame_t *frames = (const frame_t *)NULL;

int frame_index = USER_MEM_START/sizeof(frame_t);


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

