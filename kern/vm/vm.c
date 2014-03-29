/** @file vm.c
 *
 *  @brief Implements out virtual memory allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

/* VM includes */
#include <vm.h>

/* Pebbles includes */
#include <page_alloc.h>
#include <frame_alloc.h>
#include <util.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>

/* ZFOD includes */
#include <frame_alloc.h>

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
int mreg_insert(cll_list *map, mem_region_s *new)
{
  cll_node *n, *p;
  mem_region_s *mreg;
  ord_e ord;

  /* Try to allocate a cll node */
  n = malloc(sizeof(cll_node));
  if (!n) return -1;

  /* Ordered insertion by address into the mem map*/
  cll_foreach(map, p){
    mreg = cll_entry(mem_region_s *, p);
    ord = mreg_compare(new, mreg);
    if((ord == ORD_LT) || (ord == ORD_EQ))
      break;
  }

  cll_init_node(n, new);
  cll_insert(p, n);

  return 0;
}

mem_region_s *mreg_extract(cll_list *map, mem_region_s *targ)
{
  cll_node *n;
  mem_region_s *mreg;

  cll_foreach(map, n) {
    mreg = cll_entry(mem_region_s *, n);
    if (mreg_compare(mreg, targ) == ORD_EQ) {
      cll_extract(map, n);
      free(n);
      return mreg;
    }
  }

  return NULL;
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


/*************************************************************************
 *  The actual vm allocator
 *************************************************************************/

/** @brief Intitialize the VM sub-system.
 *
 *  @return Void.
 **/
void vm_init_allocator(void)
{
  pg_init_allocator();

  return;
}

/*  @brief Allocates region struct but no physical frames to back it
 *
 *  This function allocates a region of virtual memory in the address-space
 *  specified by vmi,
 *
 *  The starting address, va_start, is rounded down to a page boundary to
 *  determine the actual starting address for the allocation.  Furthermore,
 *  only whole pages are allocated, so (va_start + len) is rounded up to a
 *  page bondary as well.  Obviously, the actual allocation may be larger
 *  than the reqested space.  
 *
 *  @param vmi The vm_info struct for this allocation.
 *  @param va_start The requested starting address for the allocation.
 *  @param len The length (in bytes) of the allocation.
 *  @param attrs Attributes the allocated region will have.
 *
 *  @return NULL on error or the region allocated
 **/
void *vm_region(vm_info_s *vmi, void *va_start, size_t len,
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
  if (pg_count > frame_remaining()) {
    return NULL;
  }

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
  return mreg;
}
 
/** @brief Allocate a contiguous region in memory.
 *
 *  The new region will starting at va_start, extending for len bytes, and have
 *  the protections specified by attrs. The region struct is allocated by
 *  vm_region(), and the region is backed by physical frames in this function.
 *
 *  If the allocation succeeds, the actual starting address of the allocated
 *  region is returned; otherwise NULL is returned.
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
  void *addr, *addr2;
  mem_region_s *mreg;

  /* Allocate a region */
  mreg = vm_region(vmi, va_start, len, attrs);
  if(!mreg) return NULL;

  /* Allocate frames for the requested memory */
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    if (!alloc_page(&vmi->pg_info, addr, attrs)) {
      for (addr2 = mreg->start; addr < addr; addr += PAGE_SIZE)
        free_page(&vmi->pg_info, addr);
      return NULL;
    }
  }

  /* Return the ACTUAL start of the allocation */
  return mreg->start;
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
    if (page_set_attrs(&vmi->pg_info, addr, attrs))
      return -1;
  }

  mreg->attrs = attrs;
  return 0;
}

/** @brief Frees a region's page tables if they are not shared by neighbor
 * regions.
 *
 *  @param map Memory map to search.
 *  @param targ Region to check.
 *
 *  @return Void.
 **/
void vm_boundaries(vm_info_s *vmi, mem_region_s *targ)
{
  cll_node *n;
  mem_region_s *curr, *prev, *next;
  int targ_lo, targ_hi, i;

  /* Make these impossible values for the case when you have no neighbors */
  int index_lo = -1;
  int index_hi = PG_TBL_ENTRIES;

  /* Find your neighbors */
  cll_foreach(&vmi->mmap, n){
    curr = cll_entry(mem_region_s *, n);
    if (mreg_compare(curr, targ) == ORD_EQ) {
      break;
    }
  }
  assert(cll_entry(mem_region_s *, n) == targ);

  /* Compute your bounds */
  targ_lo = PG_DIR_INDEX(targ->start);
  targ_hi = PG_DIR_INDEX(targ->limit);

  /* TODO: add/sub 1? */
  /* Extract your neighbors */

  /* There's a neighbor below you in memory */
  if(n->prev != &vmi->mmap){
    prev = cll_entry(mem_region_s *, n->prev);
    index_lo = PG_DIR_INDEX(prev->limit);
  }
  /* There's a neighbor above you in memory */
  if(n->next != &vmi->mmap){
    next = cll_entry(mem_region_s *, n->next);
    index_hi = PG_DIR_INDEX(next->start);
  }

  /* Free your lower boundary */
  if(targ_lo != index_lo)
    pg_tbl_free(&vmi->pg_info, targ->start);

  /* Free your upper boundary */
  if(targ_hi != index_hi)
    pg_tbl_free(&vmi->pg_info, targ->limit);

  /* Free anywhere in between */
  for(i = ++targ_lo; i < targ_hi; i++){
    pg_tbl_free(&vmi->pg_info, tomes[i]);
  }

  return;
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
  mreg = mreg_extract(&vmi->mmap, &temp);
  if (!mreg) return;

  /* Free pages in that region */
  for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
    free_page(&vmi->pg_info, addr);
  }

  /* Check if page tables should be freed */
  vm_boundaries(vmi, mreg);

  free(mreg);

  /* Sanity check */
  validate_pd(&vmi->pg_info);

  return;
}

/** @brief Copies an address space.
 *
 *  @param dst The destination vm_info struct.
 *  @param src The source vm_info struct.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
#include <tlb.h>
#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )
int vm_copy(vm_info_s *dst, vm_info_s *src)
{
  mem_region_s *dreg;
  const mem_region_s *sreg;
  cll_node *n;
  void *addr, *buf;

  /* Don't copy unless the dest is empty */
  if (!cll_empty(&dst->mmap)) return -1;

  /* Allocate a buffer for copying frames */
  buf = smemalign(PAGE_SIZE, PAGE_SIZE);
  if (!buf) return -1;

  /* Map in the dst tables */
  dst->pg_info.pg_tbls = CHILD_PDE;
  pte_t pde = PACK_PTE(dst->pg_info.pg_dir, PG_SELFREF_ATTRS);
  set_pde(src->pg_info.pg_dir, CHILD_PDE, &pde);

  cll_foreach(&src->mmap, n)
  {
    sreg = cll_entry(mem_region_s *, n);

    /* Allocate a dst region struct */
    dreg = vm_region(dst, sreg->start, sreg->limit-sreg->start, sreg->attrs);
    if (!dreg) {
      sfree(buf, PAGE_SIZE);
      return -1;
    }

    /* Allocate pages for the region */
    for (addr = sreg->start; addr < sreg->limit; addr += PAGE_SIZE)
      assert( !copy_page(&dst->pg_info, &src->pg_info, addr, buf) );
  }

  /* Unmap dest tables */
  pde = PACK_PTE(NULL, 0);
  set_pde(src->pg_info.pg_dir, CHILD_PDE, &pde);
  tlb_inval_tome(CHILD_PDE);
  tlb_inval_page(src->pg_info.pg_tbls[PG_DIR_INDEX(dst->pg_info.pg_tbls)]);
  dst->pg_info.pg_tbls = PG_TBL_ADDR;

  sfree(buf, PAGE_SIZE);
  return 0;
}
void vm_region_free(vm_info_s *vmi)
{
  mem_region_s *mreg;
  int prev_pdi = -1;
  int pdi;

  while ( (mreg = mreg_extract_any(&vmi->mmap)) )
  {
    pdi = PG_DIR_INDEX(mreg->start);

    /* Avoid freeing a page table twice */
    if(pdi == prev_pdi){
      ++pdi;
    }

    /* Free the page tables */
    for(;((void *)(&tomes[pdi])) < mreg->limit; pdi++){
      pg_tbl_free(&vmi->pg_info,&tomes[pdi]);
    }

    /* Store out the index of the last page table freed */
    prev_pdi = PG_DIR_INDEX(mreg->limit);

    /* Free the region */
    free(mreg);
  }

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
  void *addr;
  cll_node *n;
  cll_foreach(&vmi->mmap, n)
  {
    mreg = cll_entry(mem_region_s *, n);

    /* Free all of each region's pages */
    for (addr = mreg->start; addr < mreg->limit; addr += PAGE_SIZE) {
      free_page(&vmi->pg_info, addr);
    }
  }

  vm_region_free(vmi);

  /* Sanity check */
  validate_pd(&vmi->pg_info);

  return;
}


