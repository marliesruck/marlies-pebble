/** @file mem_mgmt.c
 *
 *  @brief Implements our memory management system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sched.h>
#include <vm.h>
#include <mutex.h>


/*************************************************************************
 *  Memory management system calls
 *************************************************************************/

#include <simics.h>
int sys_new_pages(void *addr, int len)
{
  lprintf("sys_new_pages(addr=%p, len=0x%08x)", addr, len);

  /* Parameter checking */
  if ((unsigned int) addr % PAGE_SIZE) return -1;
  if (len < 0 || len % PAGE_SIZE) return -1;

  mutex_lock(&curr->task_info->lock);

  if (!vm_alloc(&curr->task_info->vmi, addr, len,
                VM_ATTR_RDWR|VM_ATTR_USER|VM_ATTR_NEWPG))
  {
    mutex_unlock(&curr->task_info->lock);
    lprintf("  failure!");
    return -1;
  }

  mutex_unlock(&curr->task_info->lock);
  lprintf("  success!");
  return 0;
}

int sys_remove_pages(void *addr)
{
  unsigned int attrs;

  if ((unsigned int) addr % PAGE_SIZE) return -1;

  mutex_lock(&curr->task_info->lock);

  if (vm_get_attrs(&curr->task_info->vmi, addr, &attrs)
      || !(attrs & VM_ATTR_NEWPG))
  {
    mutex_unlock(&curr->task_info->lock);
    return -1;
  }

  vm_free(&curr->task_info->vmi, addr);

  mutex_unlock(&curr->task_info->lock);
  return 0;
}

