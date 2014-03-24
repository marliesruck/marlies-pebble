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
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>


/* Serialize access to the frame allocator */
mutex_s frame_allocator_lock = MUTEX_INITIALIZER(frame_allocator_lock);

/*************************************************************************
 *  Helper functions
 *************************************************************************/

void translate_attrs(pte_s *pte, unsigned int attrs)
{
  pte->writable = (attrs & VM_ATTR_RDWR) ? 1 : 0;
  pte->user = (attrs & VM_ATTR_USER) ? 1 : 0;

  return;
}

void *alloc_page_table(pg_info_s *pgi, void *vaddr)
{
  pte_s pde;

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();
  if (!frame) return NULL;

  /* Map the frame into virtual memory so we can read the address of the next
   * free frame */
  init_pte(&pde, frame);
  pde.present = 1;
  pde.writable = 1;
  pde.user = 1;
  set_pde(pgi->pg_dir, vaddr, &pde);

  /* Update the head of the free list */
  void *new_head = (void *)(pgi->pg_tbls[PG_DIR_INDEX(vaddr)][0].addr 
                    << PG_TBL_SHIFT);
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
  pte_s pde;

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
  init_pte(&pde, frame);
  pde.present = 1;
  translate_attrs(&pde, attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  /* Update the head of the free list */
  update_head_wrapper(vaddr);

  /* Relinquish the lock */
  mutex_unlock(&frame_allocator_lock);

  return frame;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

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
  pte_s pde;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, NULL)) {
    alloc_page_table(pgi, vaddr);
  }

  /* Back vaddr with the ZFOD frame */
  init_pte(&pde, zfod);
  pde.present = 1;
  pde.zfod = 1;
  pde.user = (attrs & VM_ATTR_USER) ? 1 : 0;
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  return zfod;
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
  pte_s pte;

  /* Find the page's PTE */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte))
    return -1;

  /* Write the new attributes */
  translate_attrs(&pte, attrs);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  /* Do we need an tlb_inval_page() here? */

  return 0;
}

#define BUF ( (void *)&tomes[PG_TBL_ENTRIES - 4][PG_TBL_ENTRIES - 1])
void *buffered_copy(pg_info_s *src, void *vaddr)
{
    pte_s pte;

    /* Allocate the dest page and map it into the current address 
     * space for copying */
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

/** @brief Copies an address page.
 *
 *  @param dst The destination page info struct.
 *  @param src The source page info struct.
 *  @param addr The vitual address of the page to copy.
 *  @param attrs Attributes for the copied page.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
/* TODO: make this dynamic */

#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )
int copy_page(pg_info_s *dst, pg_info_s *src, void *vaddr, unsigned int attrs)
{
  pte_s pte;

  /* The page better exist in the parent */
  if (get_pte(src->pg_dir, src->pg_tbls, vaddr, &pte))
    return -1;

  void *frame = (void *)(pte.addr << PG_TBL_SHIFT);


  /* Only copy non zfod pages*/
  if (frame != zfod)
  {
    /* Copy to a buffer first */
    void *frame = buffered_copy(src, vaddr);

    /* Preserve the attributes of the source PTE and change the frame */
    pte.addr = ((uint32_t)(frame)) >> PG_TBL_SHIFT;

    if(set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte)){
      /* Add a PDE */
      alloc_page_table(dst, vaddr);
      /* Try again */
      assert(!set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte));

    }

  }
  /* Simply copy the source PTE into the dest */
  else
  {
    if(set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte)){
      /* Add a PDE */
      alloc_page_table(dst, vaddr);
      /* Try again */
      assert(!set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte));
    }
  }

  return 0;
}
/** @brief Free a frame.
 *
 *  Adds a frame to the free list.
 *
 *  @param pgi   Page table information.
 *  @param vaddr Logical address of frame.
 *  @return Void.
 **/
void free_page(pg_info_s *pgi, void *vaddr)
{
  pte_s pte;

  /* If the PDE isn't valid, there's nothing to free */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte)) return;

  /* Make the page writable so we can store an implicit pointer to 
   * the old head */
  if(!pte.writable){
    tlb_inval_page(vaddr);
    pte.writable = 1;
    set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  }
  
  /* The implicit pointer should be stored in the lowest addressable word */
  uint32_t *floor = (uint32_t *)(FLOOR(vaddr, PAGE_SIZE));
  void *frame = (void *)(pte.addr << PG_TBL_SHIFT);

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

  /* Free the page; invalidate the tlb entry */
  init_pte(&pte, NULL);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  tlb_inval_page(vaddr);

  return;
}
/** @brief Free a frame that is not mapped in to the current address space.
 *
 *  @param frame Frame to free.
 *  @param pgi Virtual memory information.
 *  @return Void.
 **/
void free_unmapped_frame(void *frame, pg_info_s *pgi)
{
 pte_s pte, pte_buf;

 /* Grab an address to map the frame in */
 void *buf = smemalign(PAGE_SIZE,PAGE_SIZE);

 /* Store out that pte */
 assert(!get_pte(pgi->pg_dir, pgi->pg_tbls, buf, &pte_buf));

 /* Map the frame in */
 init_pte(&pte, frame);
 pte.writable = 1;
 pte.present = 1;
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
