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
#define BUF ( (void *)&tomes[PG_TBL_ENTRIES - 4][PG_TBL_ENTRIES - 1])
int copy_page(pg_info_s *dst, const pg_info_s *src, void *vaddr, unsigned int attrs)
{
  void *frame;
  pte_s pde;

  /* Allocate a buffer page to map in the dest page
   * FIXME: explicitly discarding 'const' isn't really ok...
   */
  if (get_pte(src->pg_dir, src->pg_tbls, BUF, NULL)) {
    alloc_page_table((pg_info_s *)src, BUF);
  }

  /* The page better exist in the parent */
  if (get_pte(src->pg_dir, src->pg_tbls, vaddr, &pde))
    return -1;

  /* Only copy writable pages */
  if (pde.writable)
  {
    /* Allocate the dest page */
    frame = alloc_page_really(dst, vaddr, attrs);
    if (!frame) return -1;

    /* Map in the dest page for copying */
    init_pte(&pde, frame);
    pde.present = 1;
    pde.writable = 1;
    assert( !set_pte(src->pg_dir, src->pg_tbls, BUF, &pde) );

    /* Copy data */
    memcpy(BUF, vaddr, PAGE_SIZE);

    /* Unmap the dest page */
    init_pte(&pde, NULL);
    pde.present = 0;
    assert( !set_pte(src->pg_dir, src->pg_tbls, BUF, &pde) );
    tlb_inval_page(BUF);
  }

  /* Don't copy read-only pages */
  else
  {
    /* If the PDE isn't valid, make it so */
    if (get_pte(dst->pg_dir, dst->pg_tbls, vaddr, NULL)) {
      alloc_page_table(dst, vaddr);
    }

    /* Allocate the dest page */
    assert( !set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pde) );
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
