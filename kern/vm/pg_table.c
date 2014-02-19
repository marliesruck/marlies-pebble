/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <pg_table.h>
#include <frame_alloc.h>

#include <syscall.h>

static pte_t *kern_pt;

/* Populate kern page table */
void init_kern_pt(void)
{
  int i;

  /* Allocate a page table */
  kern_pt= alloc_frame();

  /* Direct map vm to kernel physical mem */
  for(i = 0; i < PAGE_SIZE; i++){
    kern_pt[i] = i * PAGE_SIZE; 
  }

  return;
}

/* --- Page directory functions --- */
void init_pde(pde_t *pd)
{
  int i;

  /* Store physical address of kern page table */
  pd[0] = PACK_PDE(kern_pt,PG_DIR_PRESENT);

  /* Init remaining PDE to not available */
  for(i = 1; i < PAGE_SIZE; i++){
    pd[i] = !PG_DIR_PRESENT; 
  }

  return;
}

void insert_pde(pde_t *pd, void *addr, pte_t *pt, unsigned int flags)
{
  /* Compute index into page directory */
  int index = PG_DIR_INDEX(addr); 

  /* Store address of page table and attributes */
  pd[index] = PACK_PDE(addr,flags); 

  return;
}

/* --- Page table functions --- */
void init_pte(pte_t *pd)
{
  int i;

  /* Init PTE to not available */
  for(i = 0; i < PAGE_SIZE; i++){
    pd[i] = !PG_TBL_PRESENT; 
  }
  return;
}

void insert_pte(pde_t *pd, void *addr, void *frame, unsigned int flags)
{
  /* Compute index into page directory */
  int pd_index = PG_DIR_INDEX(addr); 
  int pt_index = PG_TBL_INDEX(addr); 

  pte_t *pt = GET_PT(pd[pd_index]);
  pt[pt_index] = PACK_PTE(frame,flags);

  return;
}

