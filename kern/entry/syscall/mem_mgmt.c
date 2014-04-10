/** @file mem_mgmt.c
 *
 *  @brief Implements our memory management system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include <sched.h>
#include <vm.h>
#include <mutex.h>


/*************************************************************************
 *  Memory management system calls
 *************************************************************************/

/** @brief Attempts to allocate new memory.
 *
 *  This function will attempt to allocate len bytes starting at addr in
 *  the invoking thread's task's address space (i.e. it will be visible to
 *  other threads in the same task).
 *
 *  The allocation will fail if any part of the requested region overlaps
 *  with previously allocated memory or with kernel memory.  Similarly, the
 *  allocation will fail if the system does not have enough resources to
 *  satisfy the request.
 *
 *  @param addr The starting address of the allocation.
 *  @param len The lenght of the allocaiton.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int sys_new_pages(void *addr, int len)
{
  /* Parameter checking */
  if ((unsigned int) addr % PAGE_SIZE) return -1;
  if (len < 0 || len % PAGE_SIZE) return -1;

  mutex_lock(&curr_tsk->lock);

  if (!vm_alloc(&curr_tsk->vmi, addr, len,
                VM_ATTR_RDWR|VM_ATTR_USER|VM_ATTR_NEWPG))
  {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  mutex_unlock(&curr_tsk->lock);
  return 0;
}

/** @brief Deallocates memory allocated with sys_new_pages(...).
 *
 *  If the memory region specified by addr was not allocated by a call to
 *  sys_new_pages(...), the deallocation will fail.
 *
 *  @param addr The address of the allocation to deallocate.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int sys_remove_pages(void *addr)
{
  unsigned int attrs;

  if ((unsigned int) addr % PAGE_SIZE) return -1;

  mutex_lock(&curr_tsk->lock);

  if (vm_get_attrs(&curr_tsk->vmi, addr, &attrs)
      || !(attrs & VM_ATTR_NEWPG))
  {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  vm_free(&curr_tsk->vmi, addr);

  mutex_unlock(&curr_tsk->lock);
  return 0;
}

