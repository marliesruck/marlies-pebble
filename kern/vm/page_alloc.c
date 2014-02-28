/** @file vm.c
 *
 *  @brief Implements out page allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* Pebbles */
#include <frame_alloc.h>
#include <pg_table.h>

/* Libc includes */
#include <assert.h>
#include <stddef.h>
#include <string.h>


/**
 * @bug This will overwrite an existing pte for the specified virtual
 * address.
 **/
void alloc_page(pte_s *pd, void *vaddr, unsigned int attrs)
{
  pte_s pde;
  void *frame;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pd, pg_tables, vaddr, NULL)) {
    memset(&pde, 0, sizeof(pte_s));
    pde.addr = ((unsigned int) alloc_frame()) >> 12;
    pde.present = 1;
    pde.writable = 1;
    pde.user = 1;
    set_pde(pd, vaddr, &pde);
  }

  /* Back the requested vaddr with a page */
  frame = alloc_frame();
  int temp = PACK_PTE(frame,attrs);
  assert( !set_pte(pd, pg_tables, vaddr, (pte_s *)&temp) );

  return;
}

int page_is_allocated(pte_s *pd, void *vaddr)
{
  pte_s pte;

  /* If the PDE isn't valid, the page isn't allocated */
  if (get_pte(pd, pg_tables, vaddr, &pte))
    return 0;

  return pte.present;
}

