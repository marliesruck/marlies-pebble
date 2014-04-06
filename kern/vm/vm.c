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
#include <mreg.h>
#include <page_alloc.h>
#include <tlb.h>
#include <util.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>


#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )

/*************************************************************************
 *  Rename this
 *************************************************************************/

/*  @brief Creates a new memory region.
 *
 *  The starting address, va_start, is rounded down to a page boundary to
 *  determine the actual starting address for the allocation.  Furthermore,
 *  only whole pages are allocated, so (va_start + len) is rounded up to a
 *  page bondary as well.  Obviously, the actual allocation may be larger
 *  than the reqested space.  
 *
 *  If the system does not have enough physical frames, or if some part of
 *  the requested memory region is part of a previous allocation, creation
 *  will fail.
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The requested starting address for the allocation.
 *  @param len The length (in bytes) of the allocation.
 *  @param attrs Attributes the allocated region will have.
 *
 *  @return A pointer to the new region on success; NULL on error.
 **/
mem_region_s *create_mem_region(vm_info_s *vmi, void *va_start, size_t len,
                        unsigned attrs)
{

  void *pg_start, *pg_limit;
  int pg_count;
  mem_region_s *mreg;

  pg_start = (void *)FLOOR(va_start, PAGE_SIZE);
  pg_limit = (void *)CEILING(va_start + len, PAGE_SIZE);

  assert( (((unsigned int) pg_start) & (PAGE_SIZE-1)) == 0 );
  assert( (((unsigned int) pg_limit) & (PAGE_SIZE-1)) == 0 );

  /* Check that the system has enough frames */
  pg_count = (pg_limit - pg_start)/PAGE_SIZE;
  if (pg_count > frame_remaining()) return NULL;

  /* Allocate and initialize the new region */
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

  return mreg;
}

/** @brief Destroys a memory region.
 *
 *  We also extract the memory region from the memory map and free the
 *  region's page tables where possible.
 *
 *  @param map Memory map to search.
 *  @param targ Region to check.
 *
 *  @return Void.
 **/
void destroy_mem_region(vm_info_s *vmi, mem_region_s *targ)
{
  int pdi_lo, pdi_hi, i;
  int share_lo, share_hi;
  mem_region_s *prev, *next;

  /* Retrieve your neighors in memory */
  prev = mreg_prev(&vmi->mmap, targ);
  next = mreg_next(&vmi->mmap, targ);

  /* Compute boundary page directory indicies */
  pdi_lo = PG_DIR_INDEX(targ->start);
  pdi_hi = PG_DIR_INDEX(targ->limit);

  /* Figure out which page tables are shared */
  share_lo = prev ? pdi_lo == PG_DIR_INDEX(prev->limit) : 0;
  share_hi = next ? pdi_hi == PG_DIR_INDEX(next->start) : 0;

  /* The region has only one page table */
  if(pdi_lo == pdi_hi)
  {
    /* Only free the tables if we don't share them */
    if(!share_lo && !share_hi) pg_free_table(&vmi->pg_info, targ->start);
  }

  /* The region has multiple page tables */
  else
  {
    /* Free your low table if it's not shared */
    if(!share_lo) pg_free_table(&vmi->pg_info, targ->start);

    /* Free your high table if it's not shared */
    if(!share_hi) pg_free_table(&vmi->pg_info, targ->start);

    /* Free any tables in between */
    for(i = ++pdi_lo; i < pdi_hi - 1; i++)
      pg_free_table(&vmi->pg_info, &tomes[i]);
  }

  /* Extract and free the node */
  mreg_extract(&vmi->mmap, targ);
  free(targ);
  return;
}

void map_dest_tables(vm_info_s *dst, vm_info_s *src)
{
  pte_t pde;

  /* Map in the dst tables */
  dst->pg_info.pg_tbls = CHILD_PDE;
  pde = PACK_PTE(dst->pg_info.pg_dir, PG_SELFREF_ATTRS);
  set_pde(src->pg_info.pg_dir, CHILD_PDE, &pde);

  return;
}

void unmap_dest_tables(vm_info_s *dst, vm_info_s *src)
{
  pte_t pde;

  /* Unmap dest tables */
  pde = PACK_PTE(NULL, 0);
  set_pde(src->pg_info.pg_dir, CHILD_PDE, &pde);
  tlb_inval_tome(CHILD_PDE);
  tlb_inval_page(src->pg_info.pg_tbls[PG_DIR_INDEX(dst->pg_info.pg_tbls)]);
  dst->pg_info.pg_tbls = PG_TBL_ADDR;

  return;
}

/*************************************************************************
 *  Exported functions
 *************************************************************************/

/** @brief Initialize a VM info struct.
 *
 *  @param vmi The vm info struct to initialize.
 *  @param pd Pointer to the page directory.
 *  @param pt Pointer to the page tables.
 *
 *  @return Void.
 **/
void vm_init(vm_info_s *vmi)
{
  vmi->pg_info.pg_dir = pd_init();
  vmi->pg_info.pg_tbls = PG_TBL_ADDR;
  cll_init_list(&vmi->mmap);

  return;
}

/** @brief Intitialize the VM sub-system.
 *
 *  @return Void.
 **/
void vm_init_allocator(void)
{
  pg_init_allocator();

  return;
}

/** @brief Allocate a contiguous region in memory.
 *
 *  The new region will starting at va_start, extending for len bytes, and
 *  have the protections specified by attrs. The region struct is allocated
 *  by create_mem_region(), and the region is backed by physical frames in
 *  this function.
 *
 *  If the allocation succeeds, the actual starting address of the allocated
 *  region is returned; otherwise NULL is returned.
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The requested starting address for the allocation.
 *  @param len The length (in bytes) of the allocation.
 *  @param attrs Attributes the allocated region will have.
 *
 *  @return The actual starting address of the allocation on success, or
 *  NULL if the allocation fails.
 **/
void *vm_alloc(vm_info_s *vmi, void *va_start, size_t len,
               unsigned int attrs)
{
  void *addr, *addr2;
  mem_region_s *mreg;

  /* Ensure the address is not in reserved kernel memory */
  if((va_start < (void *)USER_MEM_START)
     || (va_start > (void *) PG_TBL_ADDR))
  {
    return NULL;
  }

  /* Allocate a region */
  mreg = create_mem_region(vmi, va_start, len, attrs);
  if(!mreg) return NULL;

  /* Allocate frames for the requested memory */
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE)
  {
    if (!pg_alloc(&vmi->pg_info, addr, attrs))
    {
      for (addr2 = mreg->start; addr2 < addr; addr2 += PAGE_SIZE)
        pg_free(&vmi->pg_info, addr2);
      destroy_mem_region(vmi, mreg);
      return NULL;
    }
  }

  /* Return the ACTUAL start of the allocation */
  return mreg->start;
}

/** @brief Gets the attributes for a region.
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The starting address for the allocation.
 *  @param dst A pointer to write the attributes.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int vm_get_attrs(vm_info_s *vmi, void *va_start, unsigned int *dst)
{
  mem_region_s temp, *mreg;

  /* Find the allocated region */
  mreg_init(&temp, va_start, va_start, 0); 
  mreg = mreg_lookup(&vmi->mmap, &temp);
  if (!mreg) return -1;

  *dst = mreg->attrs;
  return 0;
}

/** @brief Sets the attributes for a region.
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The starting address for the allocation.
 *  @param attrs The new attributes.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int vm_set_attrs(vm_info_s *vmi, void *va_start, unsigned int attrs)
{
  mem_region_s temp, *mreg;
  void *addr;

  /* Find the allocated region */
  mreg_init(&temp, va_start, va_start, 0); 
  mreg = mreg_lookup(&vmi->mmap, &temp);
  if (!mreg) return -1;

  /* Free pages in that region */
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    if (pg_set_attrs(&vmi->pg_info, addr, attrs))
      return -1;
  }

  mreg->attrs = attrs;
  return 0;
}

/** @brief Copies an address space.
 *
 *  @param dst The destination vm_info struct.
 *  @param src The source vm_info struct.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int vm_copy(vm_info_s *dst, vm_info_s *src)
{
  mem_region_s *dreg;
  const mem_region_s *sreg;
  cll_node *n;
  void *addr, *addr2, *buf;

  /* Don't copy unless the dest is empty */
  if (!cll_empty(&dst->mmap)) return -1;

  /* Allocate a buffer for copying frames */
  buf = smemalign(PAGE_SIZE, PAGE_SIZE);
  if (!buf) return -1;

  /* Map in the destination page tables */
  map_dest_tables(dst, src);

  cll_foreach(&src->mmap, n)
  {
    sreg = cll_entry(mem_region_s *, n);

    /* Allocate a dst region struct */
    dreg = create_mem_region(dst, sreg->start, sreg->limit-sreg->start,
                             sreg->attrs);
    if (!dreg)
    {
      vm_final(dst);
      unmap_dest_tables(dst, src);
      sfree(buf, PAGE_SIZE);
      return -1;
    }

    /* Allocate pages for the region */
    for (addr = sreg->start; addr < sreg->limit; addr += PAGE_SIZE)
    {
      if(pg_copy(&dst->pg_info, &src->pg_info, addr, buf))
      {
        for (addr2 = sreg->start; addr2 < addr; addr2 += PAGE_SIZE)
          pg_free(&dst->pg_info, addr2);
        vm_final(dst);
        unmap_dest_tables(dst, src);
        sfree(buf, PAGE_SIZE);
        return -1;
      }
    }
  }

  /* Do cleanup and return */
  unmap_dest_tables(dst, src);
  sfree(buf, PAGE_SIZE);
  return 0;
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
    pg_free(&vmi->pg_info, addr);
  }

  /* Free the region and it's page tables */
  destroy_mem_region(vmi, mreg);

  /* Sanity check */
  validate_pd(&vmi->pg_info);

  return;
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
  int pdi, prev_pdi;
  void *addr;
  cll_node *n;

  /* Free all of each region's pages */
  cll_foreach(&vmi->mmap, n) {
    mreg = cll_entry(mem_region_s *, n); 
    for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE)
      pg_free(&vmi->pg_info, addr);
  }

  /* Free each region's page tables */
  prev_pdi = -1;
  while (!cll_empty(&vmi->mmap))
  {
    mreg = cll_entry(mem_region_s *,vmi->mmap.next); 

    /* Free each page table */
    pdi = PG_DIR_INDEX(mreg->start);
    if(pdi == prev_pdi) ++pdi;
    for( ; (void *)&tomes[pdi] < mreg->limit; pdi++)
      pg_free_table(&vmi->pg_info, &tomes[pdi]);

    /* Store out the index of the last page table freed */
    prev_pdi = PG_DIR_INDEX(mreg->limit);

    /* Free the region */
    assert(mreg_extract(&vmi->mmap, mreg));
    free(mreg);
  }

  /* Make sure we don't leak tables */
  validate_pd(&vmi->pg_info);
  return;
}

/** @brief Determine whether or not an address is mapped in a task's virtual
 * memory
 *
 *  @param vmi The virtual memory information.
 *  @param add The address to look up.
 *
 *  @return NULL if not found, else the starting address of the region.
 **/
void *vm_find(vm_info_s *vmi, void *addr)
{
  cll_node *n;
  mem_region_s *mreg;

  cll_foreach(&vmi->mmap, n) {
    mreg = cll_entry(mem_region_s *, n);
    if((addr >= mreg->start) && (addr < mreg->limit))
      return mreg->start;
  }

  return NULL;
}

