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
#include <util.h>
#include <page_alloc.h>

/* Libc includes */
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <simics.h>

typedef char frame_t[PAGE_SIZE];
const frame_t *frames = (const frame_t *)NULL;
int frame_index = USER_MEM_START/sizeof(frame_t);

/* Pointer to the head of the free list */
static void *freelist_p;

int frame_remaining(void)
{
  return machine_phys_frames() - frame_index;
}

/** @brief Initialize frame allocator.
 *
 *  Stride through user memory and our implicit free list by making each free
 *  frame point to the next free frame.
 *
 *  @return Void.
 **/
void init_frame_allocator(void)
{
  int i;
  int lim = machine_phys_frames();

  for(i = frame_index; i < lim - 1; i++){
    *((uint32_t *)frames[i]) = (uint32_t)(frames[i + 1]);
  }
 *((uint32_t *)(frames[i])) = 0;

 freelist_p = (void *)(frames[frame_index]);

 return;
}

/** @brief Retrieves head of free list.
 *
 *  Our frame allocator is implemented as an implicit free list.  This function
 *  acquires free the list lock and returns the head of the list. The allocator
 *  makes several assumptions:
 *  -The free list is locked.
 *  -The old head of the free list is stored in the new head of free list
 *  before it's mapping is invalidated. 
 *  -update_head() is called immediately after
 *
 *  @return Head of free list.
 */
void *retrieve_head(void)
{
  assert(freelist_p != zfod);
  return freelist_p;
}

/** @brief Update free list with address of newly freed frame
 *
 *  Assumes free list is locked. 
 *
 *  @param frame Address of frame.
 *  @return Void.
 **/
void update_head(void *frame)
{
  /* Update head */
  assert(frame != zfod);
  freelist_p = frame;

  return;
}
/* @brief Updates head.
 *
 * Takes a virtual address, computes the floor (the location of the implicit
 * pointer to the next free frame), and updates the head accordingly.
 *
 * @param addr.
 * @return Void.
 */
void update_head_wrapper(void *addr)
{
  /* The pointer to the next free frame is stored in the lowest 
   * addressable word */
  uint32_t *floor = (uint32_t *)(FLOOR(addr, PAGE_SIZE));

  assert(((void *)(*floor)) != zfod);
  freelist_p = (void *)(*floor);
}

