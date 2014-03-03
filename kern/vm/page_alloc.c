/** @file vm.c
 *
 *  @brief Implements out page allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <page_alloc.h>

/* Pebbles */
#include <frame_alloc.h>
#include <vm.h>

/* Libc includes */
#include <assert.h>
#include <stddef.h>
#include <string.h>


/*************************************************************************
 *  Helper functions
 *************************************************************************/

void translate_attrs(pte_s *pte, unsigned int attrs)
{
  pte->writable = (attrs & VM_ATTR_RDWR) ? 1 : 0;
  pte->user = (attrs & VM_ATTR_USER) ? 1 : 0;

  return;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

/** @brief Allocate a page of virtual memory.
 *
 *  @bug This will overwrite an existing pte for the specified virtual
 *  address.  Not sure if that's alright or not...
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to allocate.
 *  @param attrs The attributes for the allocation.
 *
 *  @return Void.
 **/
void alloc_page(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  pte_s pde;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, NULL)) {
    init_pte(&pde, alloc_frame());
    pde.present = 1;
    pde.writable = 1;
    pde.user = 1;
    set_pde(pgi->pg_dir, vaddr, &pde);
  }

  /* Back the requested vaddr with a page */
  init_pte(&pde, alloc_frame());
  pde.present = 1;
  translate_attrs(&pde, attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  return;
}

/** @brief Free a page.
 *
 *  TODO: tlb_inval_page(...) needs a header.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to free.
 *
 *  @return Void.
 **/
void tlb_inval_page(void *addr);
void free_page(pg_info_s *pgi, void *vaddr)
{
  pte_s pte;

  /* If the PDE isn't valid, there's nothing to free */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte)) return;

  /* Free the frame */
  free_frame((void *)(pte.addr << 12));

  /* Free the page; invalidate the tlb entry */
  init_pte(&pte, NULL);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  tlb_inval_page(vaddr);

  return;
}

