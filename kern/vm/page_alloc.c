/** @file page_alloc.c
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
  assert(vaddr != zfod);

  /* The implicit pointer should be stored in the lowest addressable word */
  uint32_t *floor = (uint32_t *)(FLOOR(vaddr, PAGE_SIZE));

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Retrieve and store out the old head while frame is mapped in */
  void *old_head = retrieve_head();
  *floor = (uint32_t)(old_head);

  /* Add frame to free list */
  update_head(frame);

  mutex_unlock(&frame_allocator_lock);
  return;
}

void *alloc_page_table(pg_info_s *pgi, void *vaddr)
{
  pte_t pde, pte;
  page_t *tome;
  int i;

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();
  if (!frame) {
    mutex_unlock(&frame_allocator_lock);
    return NULL;
  }

  /* Map the frame into virtual memory so we can read the address of the next
   * free frame */
  init_pte(&pde, frame);
  pde = PACK_PTE(frame, PG_TBL_ATTRS);
  set_pde(pgi->pg_dir, vaddr, &pde);

  /* Update the head of the free list */
  void *new_head = (void *)GET_ADDR(pgi->pg_tbls[PG_DIR_INDEX(vaddr)][0]);
  update_head(new_head);

  mutex_unlock(&frame_allocator_lock);

  /* Zero all PTEs */
  tome = tomes[PG_DIR_INDEX(vaddr)];
  init_pte(&pte, NULL);
  for(i = 0; i < PG_TBL_ENTRIES; i++){
    assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, &tome[i], &pte) );
  }

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
  if (get_pde(pgi->pg_dir, vaddr, NULL))
  {
    if (!alloc_page_table(pgi, vaddr))
      return NULL;
  }

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  void *frame = retrieve_head();
  if (!frame) {
    mutex_unlock(&frame_allocator_lock);
    return NULL;
  }

  /* Back vaddr with the new frame
   * We just checked the PDE; set_pte(...) shouldn't fail
   */
  pde = PACK_PTE(frame, PG_TBL_PRESENT);
  translate_attrs(&pde, attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );

  /* Update the head of the free list */
  void *new_head = *(void **)FLOOR(vaddr, PAGE_SIZE);
  update_head(new_head);

  mutex_unlock(&frame_allocator_lock);
  return frame;
}

void *copy_frame(pg_info_s *src, void *vaddr, void *buf)
{
  pte_t pte;

  /* Allocate the dest page and map it into the current address 
   * space for copying
   */
  void *frame = pg_alloc_phys(src, buf);
  if (!frame) return NULL;

  /* Copy data */
  memcpy(buf, vaddr, PAGE_SIZE);

  /* Unmap the dest page
   * We just checked the PDE; set_pte(...) shouldn't fail
   */
  pte = PACK_PTE(buf, KERN_PTE_ATTRS);
  assert( !set_pte(src->pg_dir, src->pg_tbls, buf, &pte) );
  tlb_inval_page(buf);

  return frame;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

/** @brief Intitialize the page allocator.
 *
 *  @return Void.
 **/
int pg_init_allocator(void)
{
  fr_init_allocator();
  init_kern_pt();

  /* Allocate dummy frame for admiring zeroes */
  zfod = smemalign(PAGE_SIZE, PAGE_SIZE);
  if (!zfod) return -1;
  memset(zfod,0,PAGE_SIZE);

  return 0;
}

/** @brief "Allocate" a page of virtual memory.
 *
 *  This function backs all allocations with the ZFOD dummy frame.
 *
 *  @param pgi Page table information.
 *  @param vaddr The virtual address to allocate.
 *  @param attrs The attributes for the allocation.
 *
 *  @return A pointer to the frame backing the allocated page.
 **/
void *alloc_page(pg_info_s *pgi, void *vaddr, unsigned int attrs)
{
  pte_t pde;
  unsigned int real_attrs;

  /* If the PDE isn't valid, make it so */
  if (get_pde(pgi->pg_dir, vaddr, NULL))
  {
    if (!alloc_page_table(pgi, vaddr))
      return NULL;
  }

  /* Back vaddr with the ZFOD frame
   * We just checked the PDE; set_pte(...) shouldn't fail
   */
  real_attrs = PG_TBL_PRESENT | PG_TBL_ZFOD;
  real_attrs |= (attrs & VM_ATTR_USER) ? PG_TBL_USER : 0;
  pde = PACK_PTE(zfod, real_attrs);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pde) );
  tlb_inval_page(vaddr);

  return zfod;
}

/** 
 **/
void *pg_alloc_phys(pg_info_s *pgi, void *vaddr)
{
  void *frame, *new_head;
  pte_t pte;

  /* Find the page's PTE */
  assert(!get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte));

  /* Serialize access to the free list */
  mutex_lock(&frame_allocator_lock);

  /* Grab the head of free list */
  frame = retrieve_head();
  if (!frame) {
    mutex_unlock(&frame_allocator_lock);
    return NULL;
  }

  /* Back vaddr with frame
   * We just checked the PDE; set_pte(...) shouldn't fail
   */
  pte = PACK_PTE(frame, GET_ATTRS(pte));
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte) );
  tlb_inval_page(vaddr);

  /* Save the new head */
  new_head = *(void **)FLOOR(vaddr, PAGE_SIZE);
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
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte) );
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
int copy_page(pg_info_s *dst, pg_info_s *src, void *vaddr, void *buf)
{
  pte_t pte;
  void *frame = NULL;

  /* The page better exist in the parent */
  if (get_pte(src->pg_dir, src->pg_tbls, vaddr, &pte))
    return -1;

  /* Only copy non-ZFOD pages*/
  if (GET_ADDR(pte) != zfod) {
    frame = copy_frame(src, vaddr, buf);
    if (!frame) return -1;
    pte = PACK_PTE(frame, GET_ATTRS(pte));
  }

  /* The child might need a page table */
  if (get_pde(dst->pg_dir, vaddr, NULL))
  {
    if (!alloc_page_table(dst, vaddr)) {
      if (frame && frame != zfod)
        free_frame(frame, vaddr);
      return 1;
    }
  }
  
  /* Shouldn't fail--we just checked the page table */
  assert( !set_pte(dst->pg_dir, dst->pg_tbls, vaddr, &pte) );

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
    assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte) );
    tlb_inval_page(vaddr);
  }

  /* Free the frame */
  void *frame = (void *)GET_ADDR(pte);
  if(frame != zfod) free_frame(frame, vaddr);

  /* Free the page; invalidate the tlb entry */
  init_pte(&pte, NULL);
  assert( !set_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte) );
  tlb_inval_page(vaddr);

  return;
}

/** @brief Handle page faults.
 *
 *  We try to back faulting ZFOD pages with real physical frames; nothing
 *  is done with non-ZFOD pages.
 *
 *  @param pgi Page table information.
 *  @param vaddr The faulting virtual address.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
#include <sched.h>
int pg_page_fault_handler(void *vaddr)
{
  pte_t pte;
  pg_info_s *pgi;

  /* Grab the faulter's page info */
  pgi = &curr->task_info->vmi.pg_info;

  /* Get the faulting address' PTE */
  if (get_pte(pgi->pg_dir, pgi->pg_tbls, vaddr, &pte))
    return -1;

  /* Don't back ZFOD pages */
  if (GET_ADDR(pte) != zfod)
    return -1;

  /* Try to get a real frame */
  if (!pg_alloc_phys(pgi, vaddr))
    return -1;

  /* Make the page writable and zero it */
  page_set_attrs(pgi, vaddr, GET_ATTRS(pte) | PG_TBL_WRITABLE);
  memset((void *)(FLOOR(vaddr, PAGE_SIZE)), 0, PAGE_SIZE);

  return 0;
}

void pg_tbl_free(pg_info_s *pgi, void *vaddr)
{
  pte_t pte;

  /* Ensure all PTEs in this page table are invalid */
  validate_pt(pgi, vaddr);

  /* Retrieve the PDE entry */
  get_pde(pgi->pg_dir, vaddr, &pte);

  /* Free the frame */
  void *frame = (void *)GET_ADDR(pte);

  free_frame(frame, &pgi->pg_tbls[PG_DIR_INDEX(vaddr)][0]);

  /* Unmap the PDE and invalidate the tlb */
  init_pte(&pte, NULL);
  set_pde(pgi->pg_dir, vaddr, &pte);
  tlb_inval_page(&pgi->pg_tbls[PG_DIR_INDEX(vaddr)][PG_TBL_INDEX(vaddr)]);

  return;
}

/**********************
 * Invariant checkers *
 **********************/

/** @brief Ensure that if a PDE is present, there is a valid PTE in that page
 *  table as well.
 *
 *  Basically, we just want to ensure we are not leaking page tables.
 *
 *  @param pgi Page directory and table information.
 *
 *  @return Void.
 **/
void validate_pd(pg_info_s *pgi)
{
  int i, j, found;
  pte_t pte;

  for(i = KERN_PD_ENTRIES; i < PG_TBL_ENTRIES - 1; i++){
    /* PDE is present, make sure PTEs are present too */
    if(!get_pde(pgi->pg_dir, &tomes[i], &pte)){
      found = 0;
      /*
      lprintf("valid pde at vaddr: %p", &tomes[i]);
      lprintf("&pgi->pg_tbls[PG_DIR_INDEX(&tomes[i])][PG_TBL_INDEX(&tomes[i])]: %p",
&pgi->pg_tbls[PG_DIR_INDEX(&tomes[i])][PG_TBL_INDEX(&tomes[i])]);
      lprintf("&pd[i]: %p",&pgi->pg_dir[i]);
      */
      for(j = 0; j < PG_TBL_ENTRIES; j++){
        /* valid PTE found */
        if(!get_pte(pgi->pg_dir, pgi->pg_tbls, &tomes[i][j], &pte)){
          found = 1;
          break;
        }
      }
      /* Make sure a valid PTE was found */
      assert(found);
    }
  }
  return;
}
/** @brief Ensure that if are freeing a page table, all PTEs are invalid.
 *
 *  @param pgi Page directory and table information.
 *  @param vaddr Virtual address that maps to page table we are checking.
 *
 *  @return Void.
 **/
void validate_pt(pg_info_s *pgi, void *vaddr)
{
  pte_t pte;
  page_t *tome;
  int i;

  tome = tomes[PG_DIR_INDEX(vaddr)];

  for(i = 0; i < PG_TBL_ENTRIES - 1; i++){
    /* Ensure no PTE is valid */
    if(!get_pte(pgi->pg_dir, pgi->pg_tbls, &tome[i], &pte)){
      lprintf("vaddr %p still mapped", &tome[i]);
      lprintf("vaddr of pte: %p", 
          &pgi->pg_tbls[PG_DIR_INDEX(&tome[i])][PG_TBL_INDEX(&tome[i])]);
      lprintf("vaddr of &pd[i]: %p", 
      &pgi->pg_dir[PG_DIR_INDEX(&tome[i])]);
      assert(0);
    }
  }

  return;
}


