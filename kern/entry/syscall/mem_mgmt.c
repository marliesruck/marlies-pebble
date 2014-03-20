/** @file mem_mgmt.c
 *
 *  @brief Implements our memory management system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sched.h>
#include <vm.h>


/*************************************************************************
 *  Memory management system calls
 *************************************************************************/

int sys_new_pages(void *addr, int len)
{
  if (!vm_alloc(&curr->task_info->vmi, addr, len,
                VM_ATTR_RDWR|VM_ATTR_USER|VM_ATTR_NEWPG))
  {
    return -1;
  }

  return 0;
}

int sys_remove_pages(void *addr)
{
  return -1;
}

