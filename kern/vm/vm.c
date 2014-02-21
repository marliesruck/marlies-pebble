/** @file vm.c
 *
 *  @brief Implements out virtual memory allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* VM includes */
#include <vm.h>

/* Pebbles includes */
#include <frame_alloc.h>

/* Libc includes */
#include <assert.h>
#include <stddef.h>

#define PAGE_MASK 0xFFFFF000
#define PAGE_ALIGN(addr) (void *)(((unsigned int) (addr)) & PAGE_MASK)

/**
 * @bug This will overwrite an existing pte for the specified virtual
 * address.
 **/
void alloc_page(pte_t *pd, void *vaddr, unsigned int attr)
{
  void *frame;
  pte_t pte;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pd, pg_tables, vaddr, &pte)) {
    frame = alloc_frame();
    set_pde(pd, vaddr, frame, PG_TBL_PRESENT|PG_TBL_WRITABLE);
  }

  /* Back the requested vaddr with a page */
  frame = alloc_frame();
  assert( !set_pte(pd, pg_tables, vaddr, frame, attr) );

  return;
}

int page_is_allocated(pte_t *pd, void *vaddr)
{
  pte_t pte;

  /* If the PDE isn't valid, the page isn't allocated */
  if (get_pte(pd, pg_tables, vaddr, &pte))
    return 0;

  return pte & PG_TBL_PRESENT;
}

void *vm_alloc(pte_t *pd, void *va_start, size_t len, unsigned int attr)
{
  void *actual_start, *va_limit;
  unsigned int actual_len, num_pages;
  int i;

  actual_start = PAGE_ALIGN(va_start);
  va_limit = (void *)((unsigned int) va_start + len);
  actual_len = (unsigned int) va_limit - (unsigned int) actual_start;
  num_pages = (actual_len + PAGE_SIZE - 1)/PAGE_SIZE;

  /* Check that the requested memory is available */
  for (i = 0; i < num_pages; ++i) {
    if (page_is_allocated(pd, (char *)va_start + i*PAGE_SIZE))
      return NULL;
  }

  /* Allocate frames for the requested memory */
  for (i = 0; i < num_pages; ++i) {
    alloc_page(pd, (char *)va_start + i*PAGE_SIZE, attr);
  }

  /* Return the ACTUAL start of the allocation */
  return actual_start;
}

void vm_free(pte_t *pd, void *va_start)
{
  /* Oh, uh yeah... we just freed it. */
  return;
}

