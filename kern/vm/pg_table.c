/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <pg_table.h>
#include <frame_alloc.h>

#include <stddef.h>
#include <string.h>

static pte_s *kern_pt[KERN_PD_ENTRIES];

pt_t *pg_tables = (pt_t *)(DIR_HIGH);
pte_s *pg_dir = (pte_s *)(TBL_HIGH);

/** @brief Initialize kernel pages.
 *
 *  Kernel memory is direct-mapped into every process' lowest 16MB of
 *  memory, so we store the page tables in a global and reuse them for each
 *  process.
 *
 *  @return Void.
 **/
void init_kern_pt(void)
{
  int i, j;

  /* Direct map kernel memory */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    kern_pt[i] = alloc_frame();
    for (j = 0; j < PAGE_SIZE; j++) {
      init_pte(&kern_pt[i][j],
               (void *)(i * PAGE_SIZE * 1024 + j * PAGE_SIZE));
      kern_pt[i][j].present = 1;
      kern_pt[i][j].writable = 1;
      kern_pt[i][j].global = 1;
    }
  }

  return;
}


/*************************************************************************
 *  Page directory manipulation
 *************************************************************************/


/*************************************************************************
 *  Page directory manipulation
 *************************************************************************/

/** @brief Initialize a page directory.
 *
 *  @param pd The page table to initialize.
 *
 *  @return Void.
 **/
void init_pd(pte_s *pd)
{
  int i;

  /* Init the directory */
  init_pt(pd);

  /* Map the kernel's page table */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    init_pte(&pd[i], kern_pt[i]);
    pd[i].present = 1;
    pd[i].writable = 1;
    pd[i].global = 1;
  }

  /* The page directory is also a page table... */
  init_pte(&pd[PG_TBL_ENTRIES - 1], pd);
  pd[PG_TBL_ENTRIES - 1].present = 1;
  pd[PG_TBL_ENTRIES - 1].writable = 1;

  return;
}

/** @brief Get a page directory entry.
 *
 *  @param pd The page directory the entry is in.
 *  @param addr The virtual address whose entry we want.
 *
 *  @return The page directory entry for the specified virtual address.
 **/
pte_s get_pde(pte_s *pd, void *addr)
{
  int pdi;
  pdi = PG_DIR_INDEX(addr);
  return pd[pdi];
}

/** @brief Set a page directory entry.
 *
 *  @param pd The page directory the entry is in.
 *  @param addr The virtual address whose entry we want to write.
 *  @param pte The entry to write into the page directory.
 *
 *  @return Void.
 **/
void set_pde(pte_s *pd, void *addr, pte_s *pte)
{
  int pdi;
  pdi = PG_DIR_INDEX(addr); 
  pd[pdi] = *pte;
  return;
}


/*************************************************************************
 *  Page table manipulation
 *************************************************************************/

/** @brief Initialize a page table entry.
 *
 *  @param pt The page table entry to initialize.
 *
 *  @return Void.
 **/
void init_pte(pte_s *pte, void *frame)
{
  memset(pte, 0, sizeof(pte_s));
  pte->addr = ((unsigned int) frame) >> PG_TBL_SHIFT;
  return;
}

/** @brief Initialize a page table.
 *
 *  @param pt The page table to initialize.
 *
 *  @return Void.
 **/
void init_pt(pte_s *pt)
{
  int i;

  /* Init everything to absent */
  for(i = 0; i < PAGE_SIZE; i++) {
    init_pte(&pt[i], NULL);
  }

  return;
}

/** @brief Get a page table entry.
 *
 *  @param pd The page directory the entry's page table is in.
 *  @param pt The page table the entry is in.
 *  @param addr The virtual address whose entry we want.
 *  @param dst A pointer to into which to write the entry.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int get_pte(pte_s *pd, pt_t *pt, void *addr, pte_s *dst)
{
  int pdi, pti;
  pte_s pde;
  
  /* Return an error if the PDE isn't present */
  pde = get_pde(pd, addr);
  if (!pde.present)
    return -1;

  /* Compute index into page directory */
  pdi = PG_DIR_INDEX(addr); 
  pti = PG_TBL_INDEX(addr); 

  /* "Return" the entry */
  if (dst) *dst = pt[pdi][pti];

  return 0;
}

/** @brief Set a page table entry.
 *
 *  @param pd The page directory the entry's page table is in.
 *  @param pt The page table the entry is in.
 *  @param addr The virtual address whose entry we want to write.
 *  @param pte The entry to write into the page table.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int set_pte(pte_s *pd, pt_t *pt, void *addr, pte_s *pte)
{
  int pdi, pti;
  pte_s pde;
  
  /* Return an error if the PDE isn't present */
  pde = get_pde(pd, addr);
  if (!pde.present)
    return -1;

  /* Compute index into page directory */
  pdi = PG_DIR_INDEX(addr); 
  pti = PG_TBL_INDEX(addr); 

  /* Write to the page table */
  pt[pdi][pti] = *pte;

  return 0;
}

