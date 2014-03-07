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
#include <util.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>


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
  cll_node *n;
  mem_region_s *mreg;

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
  cll_insert(map, n);

  return 0;
}

mem_region_s *mreg_extract_any(cll_list *map)
{
  cll_node *n;
  mem_region_s *mreg;

  if (cll_empty(map)) return NULL;

  /* Extract the list node and grab it's data */
  n = cll_extract(map, map->next);
  mreg = cll_entry(mem_region_s *, n);

  /* Free the node and return */
  free(n);
  return mreg;
}


/*************************************************************************
 *  VM Information manipulation
 *************************************************************************/


/*************************************************************************
 *  The actual vm allocator
 *************************************************************************/

/** @brief Allocate a contiguous region in memory.
 *
 *  This function allocates a region of virtual memory in the address-space
 *  specified by vmi,  The new region will starting at va_start, extending
 *  for len bytes, and have the protections specified by attrs.
 *
 *  The starting address, va_start, is rounded down to a page boundary to
 *  determine the actual starting address for the allocation.  Furthermore,
 *  only whole pages are allocated, so (va_start + len) is rounded up to a
 *  page bondary as well.  Obviously, the actual allocation may be larger
 *  than the reqested space.  If the allocation succeeds, the actual
 *  starting address of the allocated region is returned; otherwise NULL is
 *  returned.
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The requested starting address for the allocation.
 *  @param len The length (in bytes) of the allocation.
 *  @param attrs Attributes the allocated region will have.
 *
 *  @return The actual starting address of the allocation.
 **/
void *vm_alloc(vm_info_s *vmi, void *va_start, size_t len,
               unsigned int attrs)
{
  void *pg_start, *pg_limit;
  mem_region_s *mreg;
  void *addr;

  pg_start = (void *)FLOOR(va_start, PAGE_SIZE);
  pg_limit = (void *)CEILING(va_start + len, PAGE_SIZE);
  assert( (((unsigned int) pg_start) & (PAGE_SIZE-1)) == 0 );
  assert( (((unsigned int) pg_limit) & (PAGE_SIZE-1)) == 0 );

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
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    alloc_page(&vmi->pg_info, addr, attrs);
  }

  /* Return the ACTUAL start of the allocation */
  return pg_start;
}

/** @brief Frees a region previously allocated by vm_alloc(...).
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The starting address for the allocation.
 *
 *  @return Void.
 **/
void vm_free(vm_info_s *vmi, void *va_start)
{
  mem_region_s temp, *mreg;
  void *addr;

  /* Find the allocated region */
  mreg_init(&temp, va_start, va_start, 0); 
  mreg = mreg_lookup(&vmi->mmap, &temp);
  if (!mreg) return;

  /* Free pages in that region */
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    free_page(&vmi->pg_info, addr);
  }

  free(mreg);
  return;
}

/** @brief Copies an address space.
 *
 *  @param dst The destination vm_info struct.
 *  @param src The source vm_info struct.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int vm_copy(vm_info_s *dst, const vm_info_s *src)
{
  mem_region_s *dreg;
  const mem_region_s *sreg;
  cll_node *n;
  void *addr;

  /* Don't copy unless the dest is empty */
  if (!cll_empty(&dst->mmap)) return -1;

  cll_foreach(&src->mmap, n)
  {
    sreg = cll_entry(mem_region_s *, n);

    /* Allocate a dst region struct */
    dreg = malloc(sizeof(mem_region_s));
    if (!dreg) {
      vm_final(dst);
      return -1;
    }

    /* Allocate pages for the region */
    for (addr = sreg->start; addr < sreg->limit; addr += PAGE_SIZE) {
      copy_page(&dst->pg_info, &src->pg_info, addr, sreg->attrs);
    }

    /* Insert the new region into the dest */
    mreg_init(dreg, sreg->start, sreg->limit, sreg->attrs);
    if (mreg_insert(&dst->mmap, dreg)) {
      vm_final(dst);
      free(dreg);
      return -1;
    }
  }

  return 0;
}

/** @brief Frees a process' entire address space.
 *
 *  Technically, this function only free those parts allocated with
 *  vm_alloc(...).
 *
 *  @param vmi The vm_info struct whose memory map we'll free.
 *
 *  @return Void.
 **/
void vm_final(vm_info_s *vmi)
{
  mem_region_s *mreg;
  void *addr;

  while ( (mreg = mreg_extract_any(&vmi->mmap)) )
  {
    /* Free all of each region's pages */
    for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
      free_page(&vmi->pg_info, addr);
    }

    /* Free the region's struct */
    free(mreg);
  }

  return;
}

