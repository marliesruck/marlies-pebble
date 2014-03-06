/** @file fork_i.c
 *
 *  @brief Implements fork.
 *
 *  @bug
 *  -We need to commit to a consist memory map where we preserve certain PDEs, I suggest:
 *  PARENT_PG_DIR
 *  ROOT THREAD USER STACK
 *
 *  CHILD_PG_DIR(this could be inefficient if parent doesn't fork)
 *
 *  KERNEL PDE
 *  KERNEL PDE
 *  KERNEL PDE
 *  KERNEL PDE
 *
 *  Write now I'm sticking the CHILD_PDE awkwardly in the middle of the heap
 *  probably...
 *
 *  -Storing out callee saved registers and compute old %ebp then restoring
 *  them?
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include "fork_i.h"

#include <x86/cr.h>

#include <malloc.h>
#include <string.h>

#include <process.h>
#include <thread.h>
#include <frame_alloc.h>
#include <copy_vm.h>

#include <simics.h>

/* @brief Copy the parent's page directory and tables to the child's address
 * space
 *
 * Map the child's page directory and tables into the parent's address space in
 * order to manipulate the entries
 */
void *mem_map_child(void)
{
  /* Change pg_dir name to make the manipulations clearer */
  pte_s *parent_pd = pg_dir; 

  /* Allocate frame for child's page directory */
  void *child_pd = alloc_frame();

  /* Add child PDE in parent pd */
  parent_pd[PG_DIR_INDEX(CHILD_PDE)].present = 1;
  parent_pd[PG_DIR_INDEX(CHILD_PDE)].writable = 1;
  parent_pd[PG_DIR_INDEX(CHILD_PDE)].addr =  FRAME(child_pd);

  /* Make the last PDE in child's directory self-referential */
  pg_tables[PG_DIR_INDEX(CHILD_PDE)][PG_TBL_ENTRIES - 1].present = 1;
  pg_tables[PG_DIR_INDEX(CHILD_PDE)][PG_TBL_ENTRIES - 1].writable = 1;
  pg_tables[PG_DIR_INDEX(CHILD_PDE)][PG_TBL_ENTRIES - 1].addr = FRAME(child_pd);

  /* Create abstractions for manipulating the page directory and table entries */
  pt_t *child_pg_tables = (pt_t *)(CHILD_PDE);
  pte_s *my_pd = (pte_s *)(child_pg_tables[PG_TBL_ENTRIES - 1]);

  /* Copy parent's pd to child's pd */
  copy_pg_dir(parent_pd,pg_tables, my_pd,child_pg_tables);

  return child_pd;
}
/* @brief Allocate memory for child's pcb
 *
 * This function is responsible for:
 * -Grab a tid (atomically)
 * -Store the tid/cr3 in the new pcb
 *
 * @bug For the sake of debugging I'm just using a global pcb2 declared in
 * process.h.  However, this is certainly a function we should study when
 * contemplating our locking strategy. We should probably add a state field to
 * our tcb in case the parent forks and them immediately waits on the child.  I
 * imagine the code might be reminiscent of thread_fork and setting the NASCENT
 * state
 *
 * @param child_cr3 Physical address of child's page directory
 * @return Address of malloced child pcb 
 */
void *init_child_tcb(void *child_cr3)
{
  /* Initialize vm struct */
  task2.vmi = (vm_info_s) {
    .pg_info = (pg_info_s) {
      .pg_dir = (pte_s *)(TBL_HIGH),
      .pg_tbls = (pt_t *)(DIR_HIGH),
    },
    .mmap = CLL_LIST_INITIALIZER(task2.vmi.mmap)
  };

  /* Initialize pg dir and tid in prototype tcb */
  task2.cr3 = (uint32_t)(child_cr3);
  thread2.tid = 5;

  /* Set state to RUNNABLE */

  return NULL;
}
