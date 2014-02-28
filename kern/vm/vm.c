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
#include <page_alloc.h>
#include <pg_table.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>


#define PAGE_FLOOR(addr)  \
  (void *)(((unsigned int) (addr)) & PG_ADDR_MASK)

#define PAGE_CEILING(addr)  \
  (void *)(((unsigned int) (addr) + PAGE_SIZE - 1) & PG_ADDR_MASK)

/*************************************************************************
 *  Memory map and region manipulation
 *************************************************************************/

void mreg_init(mem_region_s *mreg, void *start, void *limit, unsigned int attrs)
{
  /* Intialize the mem region struct */
  mreg->start = start;
  mreg->limit = limit;
  mreg->attrs = attrs;

  return;
}

enum order { ORD_LT, ORD_EQ, ORD_GT };
typedef enum order ord_e;

ord_e mreg_compare(mem_region_s *lhs, mem_region_s *rhs)
{
  if (lhs->start < rhs->start && lhs->limit < rhs->start)
    return ORD_LT;
  else if (lhs->start > rhs->limit && lhs->limit > rhs->limit)
    return ORD_GT;

  return ORD_EQ;
}

int mreg_insert(cll_list *map, mem_region_s *mreg)
{
  cll_node *n;

  /* Try to allocate a cll node */
  n = malloc(sizeof(cll_node));
  if (!n) return -1;

  /* Insert the region into the mem map*/
  cll_init_node(n, mreg);
  cll_insert(map->next, n);

  return 0;
}


/*************************************************************************
 *  VM Information manipulation
 *************************************************************************/



/*************************************************************************
 *  The actual vm allocator
 *************************************************************************/

void *vm_alloc(vm_info_s *vmi, void *va_start, size_t len,
               unsigned int attrs)
{
  void *pg_start, *pg_limit;
  unsigned int num_pages;
  int i;

  cll_node *n;
  mem_region_s *mreg, *cursor;

  pg_start = PAGE_FLOOR(va_start);
  pg_limit = PAGE_CEILING(va_start + len);
  num_pages = (pg_limit - pg_start)/PAGE_SIZE;

  /* Initialize the memory region */
  mreg = malloc(sizeof(mem_region_s));
  if (!mreg) return NULL;
  mreg_init(mreg, pg_start, pg_limit-1, attrs);

  /* Check that the requested memory is available */
  cll_foreach(&vmi->mmap, n) {
    cursor = cll_entry(mem_region_s *, n);
    if (mreg_compare(mreg, cursor) == ORD_EQ) {
      free(mreg);
      return NULL;
    }
  }

  /* Insert it into the memory map */
  if (mreg_insert(&vmi->mmap, mreg)) {
    free(mreg);
    return NULL;
  }

  /* Allocate frames for the requested memory */
  for (i = 0; i < num_pages; ++i) {
    alloc_page(vmi->pg_dir, (char *)pg_start + i*PAGE_SIZE, attrs);
  }

  /* Return the ACTUAL start of the allocation */
  return pg_start;
}

void vm_free(pte_s *pd, void *va_start)
{
  /* Oh, uh yeah... we just freed it. */
  return;
}

