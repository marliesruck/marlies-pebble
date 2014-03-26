/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <simics.h>

/* Page table includes */
#include <pg_table.h>

/* Pebbles includes */
#include <frame_alloc.h>

/* Libc includes */
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>


page_t *pages = (page_t *)NULL;
tome_t *tomes = (tome_t *)NULL;

#define KERN_PTE_ATTRS ( PG_TBL_PRESENT | PG_TBL_WRITABLE | PG_TBL_GLOBAL )
static pte_t *kern_pt[KERN_PD_ENTRIES];


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

  for (i = 0; i < KERN_PD_ENTRIES; i++)
  {
    /* Allocate kernel page table */
    kern_pt[i] = retrieve_head();
    update_head(*(void **)kern_pt[i]);

    /* Direct map kernel pages */
    for (j = 0; j < PG_TBL_ENTRIES; j++) {
      kern_pt[i][j] = PACK_PTE(&tomes[i][j], KERN_PTE_ATTRS);
    }
  }

  return;
}


/*************************************************************************
 *  Page directory manipulation
 *************************************************************************/

/** @brief Allocate and initialize a page directory.
 *
 *  @return A pointer to the new page directory.
 **/
pte_t *pd_init(void)
{
  pte_t *pd;
  int i;

  /* Allocate memory for the page directory */
  pd = smemalign(PAGE_SIZE, PAGE_SIZE);
  if (!pd) return NULL;

  /* The page directory is also a page table... */
  pd[PG_SELFREF_INDEX] = PACK_PTE(pd, PG_SELFREF_ATTRS);

  /* Map the kernel's page table */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    pd[i] = PACK_PTE(kern_pt[i], KERN_PTE_ATTRS);
  }

  /* Init the directory */
  for (i = KERN_PD_ENTRIES; i < PG_TBL_ENTRIES; i++) {
    if (i == PG_SELFREF_INDEX) continue;
    init_pte(&pd[i], NULL);
  }

  return pd;
}

/** @brief Get a page directory entry.
 *
 *  @param pd The page directory the entry is in.
 *  @param addr The virtual address whose entry we want.
 *
 *  @return The page directory entry for the specified virtual address.
 **/
int get_pde(pte_t *pd, void *addr, pte_t *dst)
{
  int pdi;
  pdi = PG_DIR_INDEX(addr);

  /* Return an error if the PDE isn't present */
  if ( !(pd[pdi] & PG_TBL_PRESENT) )
    return -1;

  /* "Return" the PDE and return */
  if (dst) *dst = pd[pdi];
  return 0;
}

/** @brief Set a page directory entry.
 *
 *  @param pd The page directory the entry is in.
 *  @param addr The virtual address whose entry we want to write.
 *  @param pte The entry to write into the page directory.
 *
 *  @return Void.
 **/
void set_pde(pte_t *pd, void *addr, pte_t *pte)
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
void init_pte(pte_t *pte, void *frame)
{
  *pte = PACK_PTE(frame, 0);
  return;
}

/** @brief Initialize a page table.
 *
 *  @param pt The page table to initialize.
 *
 *  @return Void.
 **/
void init_pt(pte_t *pt)
{
  int i;

  /* Init everything to absent */
  for(i = 0; i < PG_TBL_ENTRIES; i++) {
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
int get_pte(pte_t *pd, pt_t *pt, void *addr, pte_t *dst)
{
  int pdi, pti;
  
  /* Return an error if the PDE isn't present */
  if (get_pde(pd, addr, NULL))
    return -1;

  /* Compute index into page directory */
  pdi = PG_DIR_INDEX(addr); 
  pti = PG_TBL_INDEX(addr); 

  /* Return an error if the PTE isn't present */
  if ( !(pt[pdi][pti] & PG_TBL_PRESENT) )
    return -1;

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
int set_pte(pte_t *pd, pt_t *pt, void *addr, pte_t *pte)
{
  int pdi, pti;
  
  /* Return an error if the PDE isn't present */ 
  if (get_pde(pd, addr, NULL))
    return -1;

  /* Compute index into page directory */
  pdi = PG_DIR_INDEX(addr); 
  pti = PG_TBL_INDEX(addr); 

  /* Write to the page table */
  pt[pdi][pti] = *pte;

  return 0;
}

