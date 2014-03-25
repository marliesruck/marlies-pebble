/** @file vm.c
 *
 *  @brief Implements out page allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include <page_alloc.h>

/* Pebbles */
#include <frame_alloc.h>
#include <tlb.h>
#include <vm.h>
#include <util.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>


/* ZFOD dummy frame */
void *zfod = NULL;

/* TODO: make these dynamic */
#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )
#define BUF ( (void *)&tomes[PG_TBL_ENTRIES - 4][PG_TBL_ENTRIES - 1])


/*************************************************************************
 *  Helper functions
 *************************************************************************/

void translate_attrs(pte_t *pte, unsigned int attrs)
{
  *pte = (attrs & VM_ATTR_RDWR)
         ? (*pte) | PG_TBL_WRITABLE
         : (*pte) & ~PG_TBL_WRITABLE;
  *pte = (attrs & VM_ATTR_USER)
         ? (*pte) | PG_TBL_USER
         : (*pte) & ~PG_TBL_USER;

  return;
}

void free_frame(void *frame, void *vaddr)
{
  /* The implicit pointer should be stored in the lowest addressable word */
  uint32_t *floor = (uint32_t *)(FLOOR(vaddr, PAGE_SIZE));

  /* Don't add the zfod frame to the free list */
  if(frame != zfod){

    /* Serialize access to the free list */
    mutex_lock(&frame_allocator_lock);

    /* Retrieve and store out the old head while frame is mapped in */
    void *old_head = retrieve_head();
    *floor = (uint32_t)(old_head);

    /* Add frame to free list */
    update_head(frame);

    /* Relinquish the lock */
    mutex_unlock(&frame_allocator_lock);
  }

  return;
}

void *alloc_page_table(pg_info_s *pgi, void *vaddr)
{
  pte_t pde;

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();
  if (!frame) return NULL;

  /* Map the frame into virtual memory so we can read the address of the next
   * free frame */
  init_pte(&pde, frame);
  pde = PACK_PTE(frame, PG_TBL_ATTRS);
  set_pde(pgi->pg_dir, vaddr, &pde);

  /* Update the head of the free list */
  void *new_head = (void *)GET_ADDR(pgi->pg_tbls[PG_DIR_INDEX(vaddr)][0]);
  update_head(new_head);

  /* Relinquish the lock */
  mutex_unlock(&frame_allocator_lock);

  return frame;
}

/** @brief Allocate a page of virtual memory.
 *
 *  This function backs all allocations with a new frame instead of using
 *  the ZFOD dummy frame.  It retrieves a frame from the free list, maps it into
 *  virtual memory, and updates the free list to point to the new head.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to allocate.
 *  @param attrs The attributes for the allocation.
 *
 *  @return Void.
 **/
void *alloc_page_really(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  pte_t pde;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, NULL)) {
    alloc_page_table(pgi, vaddr);
  }

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();
  if (!frame) return NULL;

  /* Back vaddr with the new frame */
  pde = PACK_PTE(frame, PG_TBL_PRESENT);
  translate_attrs(&pde, attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  /* Update the head of the free list */
  void *new_head = *(void **)(FLOOR(vaddr, PAGE_SIZE));
  update_head(new_head);

  /* Relinquish the lock */
  mutex_unlock(&frame_allocator_lock);

  return frame;
}

void *buffered_copy(pg_info_s *src, void *vaddr)
{
  pte_t pte;

  /* Allocate the dest page and map it into the current address 
   * space for copying
   */
  void *frame = alloc_page_really(src, BUF, VM_ATTR_RDWR);
  if (!frame) return NULL;

  /* Copy data */
  memcpy(BUF, vaddr, PAGE_SIZE);

  /* Unmap the dest page */
  init_pte(&pte, NULL);
  assert( !set_pte(src->pg_dir, src->pg_tbls, BUF, &pte) );
  tlb_inval_page(BUF);

  return frame;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

/** @brief Intitialize the page allocator.
 *
 *  @return Void.
 **/
void pg_init_allocator(void)
{
  fr_init_allocator();
  init_kern_pt();

  /* Allocate dummy frame for admiring zeroes */
  zfod = smemalign(PAGE_SIZE, PAGE_SIZE);
  memset(zfod,0,PAGE_SIZE);

  return;
}

/** @brief "Allocate" a page of virtual memory.
 *
 *  This function backs all allocations with the ZFOD dummy frame.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to allocate.
 *  @param attrs The attributes for the allocation.
 *
 *  @return Void.
 **/
void *alloc_page(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  pte_t pde;
  unsigned int real_attrs;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, NULL)) {
    alloc_page_table(pgi, vaddr);
  }

  /* Back vaddr with the ZFOD frame */
  real_attrs = PG_TBL_PRESENT | PG_TBL_ZFOD;
  real_attrs |= (attrs & VM_ATTR_USER) ? PG_TBL_USER : 0;
  pde = PACK_PTE(zfod, real_attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  return zfod;
}

/** 
 **/
void *pg_alloc_phys(pg_info_s *pgi, void *vaddr)
{
  void *frame, *new_head;
  pte_t pte;

  /* If there's no PDE, we can't back the address */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte))
    return NULL;

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  frame = retrieve_head();
  if (!frame) return NULL;

  /* Back vaddr with frame */
  pte = PACK_PTE(frame, GET_ATTRS(pte));
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte) );
  tlb_inval_page(vaddr);

  /* Save the new head */
  new_head = *(void **)(FLOOR(vaddr, PAGE_SIZE));
  update_head(new_head);

  mutex_unlock(&frame_allocator_lock);
  return frame;
}

/** @brief Sets a page's attributes.
 *
 *  @param pgi Page table information.
 *  @param vaddr A virtual address on the page.
 *  @param attrs The attributes for the page.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int page_set_attrs(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  pte_t pte;

  /* Find the page's PTE */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte))
    return -1;

  /* Write the new attributes */
  translate_attrs(&pte, attrs);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  /* Do we need an tlb_inval_page() here? */

  return 0;
}

/** @brief Copies an address page.
 *
 *  @param dst The destination page info struct.
 *  @param src The source page info struct.
 *  @param addr The vitual address of the page to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_page(pg_info_s *dst, pg_info_s *src, void *vaddr)
{
  void *frame;
  pte_t pte;

  /* The page better exist in the parent */
  if (get_pte(src->pg_dir, src->pg_tbls, vaddr, &pte))
    return -1;

  /* Only copy non zfod pages*/
  if (GET_ADDR(pte) != zfod) {
    frame = buffered_copy(src, vaddr);
    pte = PACK_PTE(frame, GET_ATTRS(pte));
  }

  /* Map the new frame into the child */
  if(set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte)) {
    alloc_page_table(dst, vaddr);
    assert( !set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte) );
  }

  return 0;
}

/** @brief Free a page.
 *
 *  @param pgi   Page table information.
 *  @param vaddr Logical address of frame.
 *  @return Void.
 **/
void free_page(pg_info_s *pgi, void *vaddr)
{
  pte_t pte;

  /* If the PDE isn't valid, there's nothing to free */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte))
    return;
  
  /* Make the page writable so we can store an implicit pointer to 
   * the old head
   */
  if( !(pte & PG_TBL_WRITABLE) ){
    pte |= PG_TBL_WRITABLE;
    set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
    tlb_inval_page(vaddr);
  }

  /* Free the frame */
  void *frame = (void *)GET_ADDR(pte);
  free_frame(frame, vaddr);

  /* Free the page; invalidate the tlb entry */
  init_pte(&pte, NULL);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  tlb_inval_page(vaddr);

  return;
}
/** @brief Free a frame that is not mapped in to the current address space.
 *
 *  @param frame Frame to free. Assumes frame is page aligned.
 *  @param pgi Virtual memory information.
 *  @return Void.
 **/
void free_unmapped_frame(void *frame, pg_info_s *pgi)
{
 pte_t pte, pte_buf;
 unsigned int attrs;

 /* Grab an address to map the frame in */
 void *buf = smemalign(PAGE_SIZE,PAGE_SIZE);

 /* Store out that pte */
 assert(!get_pte(pgi->pg_dir, pgi->pg_tbls, buf, &pte_buf));

 attrs = PG_TBL_PRESENT | PG_TBL_WRITABLE;
 pte = PACK_PTE(frame, attrs);

 assert(!set_pte(pgi->pg_dir, pgi->pg_tbls, buf, &pte));
 tlb_inval_page(buf);

 /* Add to free list */
 mutex_lock(&frame_allocator_lock);
 *(uint32_t *)(buf) = (uint32_t)retrieve_head();
 update_head(frame);
 mutex_unlock(&frame_allocator_lock);

 /* Restore and free smemaligned frame */
 assert(!set_pte(pgi->pg_dir, pgi->pg_tbls, buf, &pte_buf));
 tlb_inval_page(buf);
 sfree(buf, PAGE_SIZE);
}
