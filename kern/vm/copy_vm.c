/** @file copy_vm.c
 *
 *  @brief Implements page table-/directory- copying functionality
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <copy_vm.h>
#include <pg_table.h>
#include <frame_alloc.h>

#include "../entry/syscall/fork_i.h" /* For CHILD_PDE, get rid of this */

#include <string.h>
#include <stdlib.h>

/* Map child's frame to its parent's virtual address space so we
 * can memcpy */
#define BUF (void *)(0xE0000000)

static pte_s *kern_pt[KERN_PD_ENTRIES];

/* For striding through virtual memory */
tome_t *tomes = (tome_t *)(NULL);

                        /* --- PD/PT Copying --- */
/* @brief Copy page 
 *
 * In order to a copy a page to another address space, we map a frame into the
 * address space of the page to be copied, memcpy the page, and then return the
 * address of the frame.
 *
 * @param pt Page table to map frame to 
 * @param src Page to be copied
 * @param frame Frame to copy page to
 */

void copy_pg(pt_t *pt, void *src, void *frame)
{
  pt[PG_DIR_INDEX(BUF)][PG_TBL_INDEX(BUF)].present = 1; 
  pt[PG_DIR_INDEX(BUF)][PG_TBL_INDEX(BUF)].writable= 1; 
  pt[PG_DIR_INDEX(BUF)][PG_TBL_INDEX(BUF)].addr = (unsigned int)(frame) >> PG_TBL_SHIFT;

  /* Map frame in */
  memcpy(BUF, src, PAGE_SIZE);

  return;
}
/* @brief Copy page directory or table entry
 *
 * Assumes frame has not yet been allocated 
 *
 * @param src Page directory or table to copy from
 * @param dest Page directory or table to copy to
 * @param i Index into page directory of page table
 *
 * @return Physical address of allocated frame
 */
void *copy_pte(pte_s *src, pte_s *dest, int i)
{
   /* Allocate frame */
   void *frame = alloc_frame();

   /* Store in child PDE */
   dest[i].present = src[i].present;
   dest[i].writable= src[i].writable;
   dest[i].user = src[i].user;
   dest[i].addr = ((unsigned int) (frame)) >> PG_TBL_SHIFT;

   return frame;
}
/* Copy page directory */
/* @bug unmap child pde and buf when done!
 */
void copy_pg_dir(pte_s *src, pt_t *src_pg_tbl, pte_s *dest, pt_t *dest_pg_tbl)
{
  int i,j;

  /* Allocate a page table we reserve for copying a page from the parent's to
   * the child's address space.  This requires mapping the frame in, memcpying,
   * and then mapping the frame into the child's address space */
  void *frame = alloc_frame();
  src[PG_DIR_INDEX(BUF)].present = 1;
  src[PG_DIR_INDEX(BUF)].writable = 1;
  src[PG_DIR_INDEX(BUF)].addr = (unsigned int)(frame) >> PG_TBL_SHIFT;

  /* Map the kernel's page table */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    dest[i].present = 1;
    dest[i].writable = 1;
    dest[i].addr = ((unsigned int)(kern_pt[i])) >> PG_TBL_SHIFT;
  }

  /* Copy page directory parent's page directory.
   * Stop at PG_TBL_ENTRIES - 1 to preserve your self-referential PDE */
  for(i = KERN_PD_ENTRIES; i < PG_TBL_ENTRIES - 1; i++){
   /* Don't copy the child page directory entry */
   if(i == PG_DIR_INDEX(CHILD_PDE)){
     dest[i].present = !PG_TBL_PRESENT;
     continue;
   }
   if(src[i].present){
     /* Copy page directory entry and allocate page */
     copy_pte(src,dest,i);
     /* Copy page table */
     for(j = 0; j < PG_TBL_ENTRIES; j++){
        if(src_pg_tbl[i][j].present){
          /* Copy page table entry into child and allocate page */
          frame = copy_pte(src_pg_tbl[i],dest_pg_tbl[i], j);
          /* Copy page pointed to */
          copy_pg(src_pg_tbl, tomes[i][j],frame);
        }
        else
          dest_pg_tbl[i][j].present = !PG_TBL_PRESENT;
     }
   }
   else
     dest[i].present = !PG_TBL_PRESENT;
  }
  return;
}
