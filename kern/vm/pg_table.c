/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <pg_table.h>
#include <frame_alloc.h>

#include <string.h>

static pte_s *kern_pt[KERN_PD_ENTRIES];

pt_t *pg_tables = (pt_t *)(DIR_HIGH);
pte_s *pg_dir = (pte_s *)(TBL_HIGH);


void copy_pg_dir(pte_s *src, pte_s *dest){
  return;
}
void copy_pde(){
  return;
}

/* Populate kern page table */
void init_kern_pt(void)
{
  int i, j;

  /* Direct map kernel memory */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    kern_pt[i] = alloc_frame();
    for (j = 0; j < PAGE_SIZE; j++) {
      kern_pt[i][j].addr = (i * PAGE_SIZE * 1024 + j * PAGE_SIZE) >> 12;
      kern_pt[i][j].present = 1;
      kern_pt[i][j].writable = 1;
      kern_pt[i][j].global = 1;
    }
  }
  

  return;
}

void old_init_kern_pt(void)
{
  void *frame;
  unsigned int flags;
  unsigned int temp;
  int i, j;

  /* Direct map kernel memory */
  flags = PG_TBL_PRESENT | PG_TBL_WRITABLE;
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    kern_pt[i] = alloc_frame();
    for (j = 0; j < PAGE_SIZE; j++) {
      frame = (void *)(i * PAGE_SIZE * 1024 + j * PAGE_SIZE);
      temp = PACK_PTE(frame, flags);
      kern_pt[i][j] = *(pte_s *)&temp;
    }
  }
  

  return;
}

/* --- PDE getters and setters --- */
pte_s get_pde(pte_s *pd, void *addr)
{
  int pdi;
  pdi = PG_DIR_INDEX(addr);
  return pd[pdi];
}

void set_pde(pte_s *pd, void *addr, pte_s *pt)
{
  int pdi;
  pdi = PG_DIR_INDEX(addr); 
  pd[pdi] = *pt;
  return;
}

/* --- PTE getters and setters --- */
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

/* --- PD and PT initialization --- */
void init_pt(pte_s *pt)
{
  int i;

  /* Init everything to absent */
  for(i = 0; i < PAGE_SIZE; i++){
    memset(&pt[i], 0, sizeof(pte_s));
  }
  return;
}

void init_pd(pte_s *pd)
{
  int i;

  /* Init the directory */
  init_pt(pd);

  /* Map the kernel's page table */
  for (i = 0; i < KERN_PD_ENTRIES; i++) {
    pd[i].addr = ((unsigned int) kern_pt[i]) >> 12;
    pd[i].present = 1;
    pd[i].writable = 1;
    pd[i].global = 1;
  }

  /* The page directory is also a page table... */
  pd[PG_TBL_ENTRIES - 1].addr = ((unsigned int) pd) >> 12;
  pd[PG_TBL_ENTRIES - 1].present = 1;
  pd[PG_TBL_ENTRIES - 1].writable = 1;

  return;
}

