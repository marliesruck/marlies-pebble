/** @file vm.c
 *
 *  @brief Implements out virtual memory allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <simics.h>

#include <vm.h>

#include <stddef.h>

#define PAGE_MASK 0xFFFFF000
#define PAGE_ALIGN(addr) (void *)(((unsigned int) (addr)) & PAGE_MASK)

void *vm_alloc(pte_t *pd, void *addr, size_t len)
{
  void *pg_start;
  unsigned int num_pages;

  lprintf("vm_alloc(pd=%p, addr=%p, len=%d)", pd, addr, len);

  pg_start = PAGE_ALIGN(addr);
  lprintf("  aligned = %p", pg_start);
  num_pages = ((unsigned int) addr + len);
  lprintf("  last = 0x%08X", num_pages);
  num_pages /= PAGE_SIZE;
  lprintf("  num_pages = %u", num_pages);

  return NULL;
}

void vm_free(pte_t *pd, void *addr)
{
  return;
}

