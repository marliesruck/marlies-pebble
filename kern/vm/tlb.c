/** @file tlb.c
 *
 *  @brief Implements TLB-related functionality.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs.
 */

#include "tlb.h"

#include <pg_table.h>
#include <page_alloc.h>


/** @brief Invalidates page table-worth of TLB entries.
 *
 *  A note on terminology: we use the term "tome" to refer to a the
 *  contiguous region of memory handled by a single page table.  (It's a
 *  silly term I know, but I wasn't sure what to call it.)  In the case of
 *  4KB page tables and 4 byte entries, a "tome" is a 4MB region of memory,
 *  aligned to a 4MB boundary.
 *
 *  @param pd The page directory containing the page table.
 *  @param pt The page table whose TLB entries we'll invalidate.
 *
 *  @return Void.
 **/
void tlb_inval_tome(void *pt)
{
  page_t *tome;
  int i;

  tome = tomes[PG_DIR_INDEX(pt)];
  for (i = 0; i < PG_TBL_ENTRIES; ++i) {
    tlb_inval_page(&tome[i]);
  }

  return;
}

/** @brief Invalidate a self referential PDE mapping
 *
 *  @param pgi Page table information.
 *  @param vaddr The faulting virtual address.
 *
 *  @return Void.
 **/
void tlb_inval_pde(pg_info_s *pgi, void *vaddr)
{
  tlb_inval_page(&pgi->pg_tbls[PG_DIR_INDEX(vaddr)][PG_TBL_INDEX(vaddr)]);

  return;
}
