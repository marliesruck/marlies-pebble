/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <pg_table.h>

#include <frame_alloc.h>


#define KERN_PD_ENTRIES PG_DIR_INDEX(USER_MEM_START)
static pte_t *kern_pt[KERN_PD_ENTRIES];

pt_t *pg_tables = (pt_t *)USER_MEM_START;
pde_t *pg_directory = (pde_t *)(pt_t *)(USER_MEM_START + PAGE_SIZE*4);

/* Populate kern page table */
void init_kern_pt(void)
{
  void *frame;
  unsigned int flags;
  int i, j;

  /* Direct map kernel memory */
  flags = PG_TBL_PRESENT | PG_TBL_WRITABLE;
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    kern_pt[i] = alloc_frame();
    for (j = 0; j < PAGE_SIZE; j++) {
      frame = (void *)(i * PAGE_SIZE * 1024 + j * PAGE_SIZE);
      kern_pt[i][j] = PACK_PTE(frame, flags);
    }
  }
  

  return;
}

/* --- PDE getters and setters --- */
pde_t get_pde(pde_t *pd, void *addr)
{
  int index;
  index = PG_DIR_INDEX(addr);
  return pd[index];
}

void set_pde(pde_t *pd, void *addr, pte_t *pt, unsigned int flags)
{
  /* Compute index into page directory */
  int index = PG_DIR_INDEX(addr); 

  /* Store address of page directory and attributes */
  pd[index] = PACK_PDE(pt,flags); 

  return;
}

/* --- PTE getters and setters --- */
int get_pte(pde_t *pd, void *addr, pte_t *dst)
{
  int index;
  pde_t pde;
  pte_t *pt;

  pde = get_pde(pd, addr);

  /* And if the PDE isn't present?? */
  if (!(pde & PG_TBL_PRESENT))
    return -1;

  pt = GET_PT(pde);
  index = PG_TBL_INDEX(addr); 

  *dst = pt[index];
  return 0;
}

int set_pte(pde_t *pd, void *addr, void *frame, unsigned int flags)
{
  pde_t pde = get_pde(pd, addr);

  /* Return an error if the PDE isn't present */
  if (!(pde & PG_TBL_PRESENT))
    return -1;

  /* Compute index into page directory */
  int index = PG_TBL_INDEX(addr); 

  /* Write to the page table */
  pte_t *pt = GET_PT(pde);
  pt[index] = PACK_PTE(frame,flags);

  return 0;
}

/* --- PD and PT initialization --- */
void init_pt(pte_t *pt)
{
  int i;

  /* Init PTE to not available */
  for(i = 0; i < PAGE_SIZE; i++){
    pt[i] = !PG_TBL_PRESENT; 
  }
  return;
}

void init_pd(pde_t *pd)
{
  int i;

  /* Init everything to absent */
  for(i = 0; i < PAGE_SIZE; i++){
    pd[i] = !PG_TBL_PRESENT; 
  }

  /* Map the kernel's page table */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    pd[i] = PACK_PDE(kern_pt[i], PG_TBL_PRESENT|PG_TBL_WRITABLE);
  }

  /* The page directory is also a page table... */
  pd[i] = PACK_PDE(pd, PG_TBL_PRESENT|PG_TBL_WRITABLE);

  return;
}

