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


mem_region_s *mreg_lookup(cll_list *map, mem_region_s *targ)
{
  mem_region_s *mreg;
  cll_node *n;

  cll_foreach(map, n) {
    mreg = cll_entry(mem_region_s *, n);
    if (mreg_compare(mreg, targ) == ORD_EQ) {
      return mreg;
    }
  }

  return NULL;
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
  mem_region_s *mreg;
  int i;

  pg_start = PAGE_FLOOR(va_start);
  pg_limit = PAGE_CEILING(va_start + len);
  num_pages = (pg_limit - pg_start)/PAGE_SIZE;

  /* Initialize the memory region */
  mreg = malloc(sizeof(mem_region_s));
  if (!mreg) return NULL;
  mreg_init(mreg, pg_start, pg_limit-1, attrs);

  /* Check that the requested memory is available */
  if (mreg_lookup(&vmi->mmap, mreg)) {
    free(mreg);
    return NULL;
  }

  /* Insert it into the memory map */
  if (mreg_insert(&vmi->mmap, mreg)) {
    free(mreg);
    return NULL;
  }

  /* Allocate frames for the requested memory */
  for (i = 0; i < num_pages; ++i) {
    alloc_page(&vmi->pg_info, (char *)pg_start + i*PAGE_SIZE, attrs);
  }

  /* Return the ACTUAL start of the allocation */
  return pg_start;
}

void vm_free(vm_info_s *vmi, void *va_start)
{
  unsigned int num_pages;
  mem_region_s temp, *mreg;
  void *addr;

  /* Find the allocated region */
  mreg_init(&temp, va_start, 0, 0); 
  mreg = mreg_lookup(&vmi->mmap, &temp);
  if (!mreg) return;

  /* Free pages in that region */
  num_pages = (mreg->start - mreg->limit)/PAGE_SIZE;
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    free_page(&vmi->pg_info, addr);
  }

  /* Oh, uh yeah... we just freed it. */
  return;
}

void vm_final(vm_info_s *vmi)
{
  mem_region_s *mreg;
  cll_node *n;
  void *addr;

  while (!cll_empty(&vmi->mmap))
  {
    /* Extract each region from the mem map */
    n = cll_extract(&vmi->mmap, vmi->mmap.next);
    mreg = cll_entry(mem_region_s *, n);

    /* Free all of each region's pages */
    for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
      free_page(&vmi->pg_info, addr);
    }
  }

  return;
}

