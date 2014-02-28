/** @file pg_table.h
 *
 *  @brief Delcares the page table API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PG_TABLE_H__
#define __PG_TABLE_H__

#include <common_kern.h>
#include <x86/page.h>


/* Page table attribute flags
 *
 * We might have also declared page *directory* flags, but opted not to
 * because our page directory is self-referential.  Therefore, it is also
 * treated as a page table.
 */
#define PG_TBL_PRESENT  0x001   /* Set indicates entry is valid */
#define PG_TBL_WRITABLE 0x002   /* Set allows writing */
#define PG_TBL_USER     0x004   /* Set allows user access */
#define PG_TBL_WRTHRU   0x008   /* Set enables write-through caching */
#define PG_TBL_NOCACHE  0x010   /* Set disables caching */
#define PG_TBL_ACCESSED 0x020   /* Set by HW when the page is accessed */
#define PG_TBL_DIRTY    0x040   /* Set by HW when page is written */
#define PG_TBL_ATTR     0x080   /* Should be unset */
#define PG_TBL_GLOBAL   0x100   /* Set prevents TLB flush on ctx switch */
#define PG_TBL_AVAIL    0xE00   /* Available for programmer use */

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
#define PG_ADDR_MASK  (PG_DIR_MASK|PG_TBL_MASK)
#define PG_ATTR_MASK  0x00000FFF
#define GET_PT(pde)       ((pte_t *)(PG_ADDR_MASK & (pde)))
#define GET_ATTRS(pde)    ((pte_t *)(PG_ATTR_MASK & (pde)))

/* Pack PDE */
#define PACK_PDE(addr,flags)  ((unsigned int)(addr) | (flags))

/* Pack PTE */
#define PACK_PTE(addr,flags)  ((unsigned int)(addr) | (flags))

#define KERN_PD_ENTRIES PG_DIR_INDEX(USER_MEM_START)
#define MEM_HIGH 0xFFFFFFFF
#define DIR_HIGH (MEM_HIGH & PG_DIR_MASK)
#define TBL_HIGH (MEM_HIGH & (PG_DIR_MASK | PG_TBL_MASK))

/** @struct page_table_entry
 *  @brief An x86 page table/directory entry.
 **/
struct page_table_entry {
  unsigned int present  : 1;    /**< Set indicates entry is valid **/
  unsigned int writable : 1;    /**< Set allows writing */
  unsigned int user     : 1;    /**< Set allows user access */
  unsigned int wr_thru  : 1;    /**< Set enables cache write-through **/
  unsigned int no_cache : 1;    /**< Set disables caching **/
  unsigned int accessed : 1;    /**< Set by HW on any access **/
  unsigned int dirty    : 1;    /**< Set by HW on write **/
  unsigned int attr     : 1;    /**< Should be unset **/
  unsigned int global   : 1;    /**< Set stops TLB flush on ctx switch **/
  unsigned int avail    : 3;    /**< Available for programmer use **/
  unsigned int addr     : 20;   /**< The frame backing this page **/
};
typedef struct page_table_entry pte_s;

#define PG_TBL_ENTRIES PAGE_SIZE/sizeof(pte_s)
typedef pte_s pt_t[PG_TBL_ENTRIES];

/* Current process' page tables and page directory */
extern pt_t *pg_tables;
extern pte_s *pg_dir;


/* --- Stuff that shouldn't be here --- */
void init_kern_pt(void);

/* --- PDE/PTE getters and setters --- */
pte_s get_pde(pte_s *pd, void *addr);
void set_pde(pte_s *pd, void *addr, pte_s *pt);

int get_pte(pte_s *pd, pt_t *pt, void *addr, pte_s *dst);
int set_pte(pte_s *pd, pt_t *pt, void *addr, pte_s *pte);

/* --- PD/PT Initialization --- */
void init_pt(pte_s *pt);
void init_pd(pte_s *pd);


#endif /* __PG_TABLE_H__ */

