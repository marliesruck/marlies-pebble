/** @file pg_table.c
 *
 *  @brief Delcares the page table API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PG_TABLE_H__
#define __PG_TABLE_H__

#include <common_kern.h>
#include <syscall.h>


/* Page table attribute flags */
#define PG_TBL_PRESENT  0x001   /* Set indicates valid */
#define PG_TBL_WRITABLE 0x002   /* Set indicates writable */
#define PG_TBL_USER     0x004   /* Set indicates user accessible */
#define PG_TBL_WRTHRU   0x008   /* Set enables write-through caching */
#define PG_TBL_NOCACHE  0x010   /* Set disables caching */
#define PG_TBL_ACCESSED 0x020   /* Set by HW when the page is accessed */
#define PG_TBL_DIRTY    0x040   /* Set by HW when page is written */
#define PG_TBL_ATTR     0x080   /* Should be unset */
#define PG_TBL_GLOBAL   0x100   /* Prevents TLB flushing on ctx switch */
#define PG_TBL_AVAIL    0xE00   /* Available for programmer use */

/* Page directory attribute flags */
#define PG_DIR_PRESENT  0x001   /* Set indicates valid */
#define PG_DIR_WRITABLE 0x002   /* Set indicates writable */
#define PG_DIR_USER     0x004   /* Set indicates user accessible */
#define PG_DIR_WRTHRU   0x008   /* Set enables write-through caching */
#define PG_DIR_NOCACHE  0x010   /* Set disables caching */
#define PG_DIR_ACCESSED 0x020   /* Set by HW when the page is accessed */
#define PG_DIR_DIRTY    0x040   /* Invalid -- leave unset */
#define PG_DIR_SIZE     0x080   /* Should be unset */
#define PG_DIR_GLOBAL   0x100   /* Invalid -- leave unset */
#define PG_DIR_AVAIL    0xE00   /* Available for programmer use */

/* Page table indexing */
#define PG_TBL_MASK    0x003FF000 /* [21,12] bits */
#define PG_TBL_SHIFT   12
#define PG_TBL_INDEX(addr)  \
  ( (PG_TBL_MASK & ((unsigned int) (addr))) >> PG_TBL_SHIFT ) 

/* Page directory indexing */
#define PG_DIR_MASK    0xFFC00000 /* [31,22] bits */
#define PG_DIR_SHIFT   22
#define PG_DIR_INDEX(addr)  \
  ( (PG_DIR_MASK & ((unsigned int) (addr))) >> PG_DIR_SHIFT ) 

/* Page directory entry unpacking */
#define PG_DIR_PT_MASK    0xFFFFF000
#define PG_DIR_ATTR_MASK  0x00000FFF
#define GET_PT(pde)       ((pte_t *)(PG_DIR_PT_MASK & (pde)))
#define GET_ATTRS(pde)    ((pte_t *)(PG_DIR_ATTR_MASK & (pde)))

/* Pack PDE */
#define PACK_PDE(addr,flags)  ((pde_t)(addr) | (flags))

/* Pack PTE */
#define PACK_PTE(addr,flags)  ((pte_t)(addr) | (flags))

typedef unsigned int pde_t;
typedef unsigned int pte_t;

#define PG_TBL_ENTRIES PAGE_SIZE/sizeof(pte_t)
typedef pte_t pt_t[PG_TBL_ENTRIES];

/* Current process' page tables and page directory */
extern pt_t *pg_tables;
extern pde_t *pg_directory;


/* --- Stuff that shouldn't be here --- */
void init_kern_pt(void);

/* --- PDE/PTE getters and setters --- */
pde_t get_pde(pde_t *pd, void *addr);
void set_pde(pde_t *pd, void *addr, pte_t *pt, unsigned int flags);
pte_t get_pte(pde_t *pd, void *addr);
int set_pte(pde_t *pd, void *addr, void *frame, unsigned int flags);

/* --- PD/PT Initialization --- */
void init_pt(pte_t *pt);
void init_pd(pde_t *pd);


#endif /* __PG_TABLE_H__ */

