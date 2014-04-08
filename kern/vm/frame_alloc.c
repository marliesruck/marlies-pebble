/** @file frame_alloc.c
 *
 *  @brief Implements our frame allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

/* Frame allocator includes */
#include <frame_alloc.h>

/* Pebbles includes */
#include <common_kern.h>
#include <util.h>
#include <page_alloc.h>

/* Libc includes */
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define FIRST_FRAME_INDEX ( USER_MEM_START/sizeof(frame_t) )

const frame_t *frames = (const frame_t *)NULL;
int fr_avail = 0;

mutex_s frame_allocator_lock = MUTEX_INITIALIZER(frame_allocator_lock);

/* Pointer to the head of the free list */
static void *freelist_p;

/** @brief Initialize frame allocator.
 *
 *  Stride through user memory and our implicit free list by making each free
 *  frame point to the next free frame.
 *
 *  @return Void.
 **/
void fr_init_allocator(void)
{
  int i;
  int lim = machine_phys_frames();

  for(i = FIRST_FRAME_INDEX; i < lim - 1; i++){
    *((uint32_t *)frames[i]) = (uint32_t)(frames[i + 1]);
    ++fr_avail;
  }
 *((uint32_t *)(frames[i])) = 0;

 freelist_p = (void *)(frames[FIRST_FRAME_INDEX]);

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
void *fr_retrieve_head(void)
{
  return freelist_p;
}

/** @brief Update free list with address of newly freed frame
 *
 *  Assumes free list is locked. 
 *
 *  @param frame Address of frame.
 *
 *  @return Void.
 **/
void fr_update_head(void *frame)
{
  freelist_p = frame;
  return;
}

