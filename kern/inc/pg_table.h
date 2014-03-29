/** @file pg_table.h
 *
 *  @brief Delcares the page table API.  *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck) **/
#ifndef __PG_TABLE_H__
#define __PG_TABLE_H__

#include <common_kern.h>
#include <x86/page.h>


/* Page table attribute flags.
 *
 * We might have also declared page *directory* flags, but opted not to
 * since they are extremely similar.  Also, we treat our page directory
 * like a table anyway (it's self-referential).
 */
#define PG_TBL_PRESENT  0x001   /**< Set indicates entry is valid **/
#define PG_TBL_WRITABLE 0x002   /**< Set allows writing **/
#define PG_TBL_USER     0x004   /**< Set allows user access **/
#define PG_TBL_WRTHRU   0x008   /**< Set enables cache write-through **/
#define PG_TBL_NOCACHE  0x010   /**< Set disables caching **/
#define PG_TBL_ACCESSED 0x020   /**< Set by HW on reference **/
#define PG_TBL_DIRTY    0x040   /**< Set by HW on write **/
#define PG_TBL_ATTR     0x080   /**< No idea; should be unset **/
#define PG_TBL_GLOBAL   0x100   /**< Set stops TLB flush on ctx switch **/
#define PG_TBL_ZFOD     0x200   /**< Set indicates ZFOD pages **/
#define PG_TBL_AVAIL    0xE00   /**< Available for programmer use **/

/* Page table masks */
#define PG_TBL_MASK    0x003FF000   /* Bits [21,12] */
#define PG_TBL_SHIFT   12
#define PG_DIR_MASK    0xFFC00000   /* Bits [31,22] */
#define PG_DIR_SHIFT   22
#define PG_ADDR_MASK  (PG_DIR_MASK|PG_TBL_MASK)
#define PG_ATTR_MASK  (~PG_ADDR_MASK)


/** @brief Calculate an address' page table index.
 *
 *  @param addr The address.
 *
 *  @return The page table index.
 **/
#define PG_TBL_INDEX(addr)  \
  ( (PG_TBL_MASK & ((unsigned int) (addr))) >> PG_TBL_SHIFT ) 

/** @brief Calculate an address' page directory index.
 *
 *  @param addr The address.
 *
 *  @return The page directory index.
 **/
#define PG_DIR_INDEX(addr)  \
  ( (PG_DIR_MASK & ((unsigned int) (addr))) >> PG_DIR_SHIFT ) 

/** @brief Retrieve the address from a page table entry.
 *
 *  @param pte The page table entry.
 *
 *  @return The address in the entry.
 **/
#define GET_ADDR(pte)   \
  ( (void *)(PG_ADDR_MASK & ((unsigned int) (pte))) )

/** @brief Retrieve the attributes from a page table entry.
 *
 *  @param pte The page table entry.
 *
 *  @return The attributes in the entry.
 **/
#define GET_ATTRS(pte)  \
  ( (unsigned int) (PG_ATTR_MASK & ((unsigned int) (pte))) )

/** @brief Construct a page table entry from an address and attributes.
 *
 *  @param addr The address for the entry.
 *  @param attrs The attributes for the entry.
 *
 *  @return The page table entry.
 **/
#define PACK_PTE(addr,attrs)  \
  ( (unsigned int)(addr) | (attrs) )

#define KERN_PD_ENTRIES PG_DIR_INDEX(USER_MEM_START)

typedef unsigned int pte_t;

#define PG_TBL_ENTRIES PAGE_SIZE/sizeof(pte_t)
typedef pte_t pt_t[PG_TBL_ENTRIES];

typedef char page_t[PAGE_SIZE];
extern page_t *pages;
typedef page_t tome_t[PG_TBL_ENTRIES];
extern tome_t *tomes;

#define TOME_SIZE (PAGE_SIZE * PG_TBL_ENTRIES)

#define PG_TBL_ATTRS ( PG_TBL_PRESENT | PG_TBL_WRITABLE | PG_TBL_USER )

#define PG_SELFREF_ATTRS ( PG_TBL_PRESENT | PG_TBL_WRITABLE )
#define PG_SELFREF_INDEX (PG_TBL_ENTRIES - 1)
#define PG_TBL_ADDR ( (pt_t *)tomes[PG_SELFREF_INDEX] )

/* Stuff that shouldn't be here */
void init_kern_pt(void);

/* Page directory operations */
pte_t *pd_init(void);
int get_pde(pte_t *pd, void *addr, pte_t *dst);
void set_pde(pte_t *pd, void *addr, pte_t *pt);

/* Page directory operations */
void init_pt(pte_t *pt);
void init_pte(pte_t *pte, void *frame);
int get_pte(pte_t *pd, pt_t *pt, void *addr, pte_t *dst);
int set_pte(pte_t *pd, pt_t *pt, void *addr, pte_t *pte);

#endif /* __PG_TABLE_H__ */

