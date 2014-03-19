/** @file vm.c
 *
 *  @brief Implements out page allocator.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <page_alloc.h>

/* Pebbles */
#include <frame_alloc.h>
#include <tlb.h>
#include <vm.h>

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
  void *frame;
  pte_s pde;

  frame = alloc_frame();
  if (!frame) return NULL;
  init_pte(&pde, frame);
  pde.present = 1;
  pde.writable = 1;
  pde.user = 1;
  set_pde(pgi->pg_dir, vaddr, &pde);

  return frame;
}

/** @brief Allocate a page of virtual memory.
 *
 *  This function backs all allocations with a new frame instead of using
 *  the ZFOD dummy frame.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to allocate.
 *  @param attrs The attributes for the allocation.
 *
 *  @return Void.
 **/
void *alloc_page_really(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  void * frame;
  pte_s pde;

  /* If the PDE isn't valid, make it so */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, NULL)) {
    alloc_page_table(pgi, vaddr);
  }

  /* Get a new frame */
  frame = alloc_frame();
  if (!frame) return NULL;

  /* Back vaddr with the new frame */
  init_pte(&pde, frame);
  pde.present = 1;
  translate_attrs(&pde, attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

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

/** @brief Free a page.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to free.
 *
 *  @return Void.
 **/
void free_page(pg_info_s *pgi, void *vaddr)
{
  pte_s pte;

  /* If the PDE isn't valid, there's nothing to free */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte)) return;

  /* Free the frame */
  free_frame((void *)(pte.addr << 12));

  /* Free the page; invalidate the tlb entry */
  init_pte(&pte, NULL);
  set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte);
  tlb_inval_page(vaddr);

  return;
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
/** @brief Acquire a frame.
 *
 * Retrieves a frame from the free list, maps it into virtual memory, and
 * updates the free list to point to the new head.
 *
 * @return NULL on error, else address of frame.
 **/
void *alloc_frame2(void)
{
  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();

  /* Map the frame into virtual memory so we can read the address of the next
   * free frame */




  /* Relinquish the lock */
  mutex_unlock(&frame_allocator_lock);
  return frame;
}
/** @brief Free a frame.
 *
 *  Adds a frame to the free list.
 *
 *  @param frame Address of frame to free.
 *  @return Void.
 **/
void free_frame2(void *frame, void *vaddr)
{
  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Retrieve and store out old head while frame is mapped in */
  void *old_head = retrieve_head();
  *(uint32_t *)(vaddr) = (uint32_t)(old_head);

  /* Add frame to free list */
  update_head(frame);

  /* Relinquish the lock */
  mutex_unlock(&frame_allocator_lock);
  return;
}
