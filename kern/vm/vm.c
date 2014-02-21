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
#include <stddef.h>

#define PAGE_MASK 0xFFFFF000
#define PAGE_ALIGN(addr) (void *)(((unsigned int) (addr)) & PAGE_MASK)

void *vm_alloc(pte_t *pd, void *va_start, size_t len, unsigned int attr)
{
  void *actual_start, *va_limit;
  unsigned int actual_len, num_pages;
  void *frame, *addr;
  pte_t pte;
  int i;

  actual_start = PAGE_ALIGN(va_start);
  va_limit = (void *)((unsigned int) va_start + len);
  actual_len = (unsigned int) va_limit - (unsigned int) actual_start;
  num_pages = (actual_len + PAGE_SIZE - 1)/PAGE_SIZE;

  /* Check that the requested memory is available */
  for (i = 0; i < num_pages; ++i)
  {
    addr = (char *)va_start + i*PAGE_SIZE;

    /* If the PDE isn't valid, make it so */
    if (get_pte(pd, pg_tables,addr, &pte)) {
      frame = alloc_frame();
      set_pde(pd, addr, frame, PG_TBL_PRESENT|PG_TBL_WRITABLE);
    }

    /* If the PTE is already in use, return an error */
    else if (pte & PG_TBL_PRESENT)
      return NULL;
  }

  /* Allocate frames for the requested memory */
  for (i = 0; i < num_pages; ++i) {
    frame = alloc_frame();
    if (set_pte(pd, pg_tables,(char *)va_start + i*PAGE_SIZE, frame, attr))
      return NULL;
  }

  /* Return the ACTUAL start of the allocation */
  return actual_start;
}

void vm_free(pte_t *pd, void *va_start)
{
  /* Oh, uh yeah... we just freed it. */
  return;
}

