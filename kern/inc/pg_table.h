/** @file pg_table.c
 *
 *  @brief Delcares the page table API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PG_TABLE_H__
#define __PG_TABLE_H__


/* Page table attribute flags */
#define PG_TBL_PRESENT  0x001   /* Set indicates valid */
#define PG_TBL_WRITABLE 0x002   /* Set indicates writable */
#define PG_TBL_USER     0x004   /* Set indicates user accessible */
#define PG_TBL_WRTHRU   0x008   /* Set enables write-through caching */
#define PG_TBL_NOCACHE  0x010   /* Set disables caching */
#define PG_TBL_ACCESSED 0x020   /* Set by HW when the page is accessed */
#define PG_TBL_DIRTY    0x040   /* Set by HW when page is written */
#define PG_TBL_ATTR     0x080   /* Should be unset */
#define PG_TBL_GLOBAL   0x100   /* Should be set */
#define PG_TBL_AVAIL    0xE00   /* Available for programmer use */

/* Page directory attribute flags */
#define PG_TBL_PRESENT  0x001   /* Set indicates valid */
#define PG_TBL_WRITABLE 0x002   /* Set indicates writable */
#define PG_TBL_USER     0x004   /* Set indicates user accessible */
#define PG_TBL_WRTHRU   0x008   /* Set enables write-through caching */
#define PG_TBL_NOCACHE  0x010   /* Set disables caching */
#define PG_TBL_ACCESSED 0x020   /* Set by HW when the page is accessed */
#define PG_TBL_DIRTY    0x040   /* Invalid -- leave unset */
#define PG_TBL_SIZE     0x080   /* Should be unset */
#define PG_TBL_GLOBAL   0x100   /* Invalid -- leave unset */
#define PG_TBL_AVAIL    0xE00   /* Available for programmer use */

typedef unsigned int pde_t;
typedef unsigned int pte_t;

void init_pde(pde_t *pd);
void init_pte(pte_t *pd);
void insert_pde(pde_t *pd, void *addr, pte_t *pt, unsigned int flags);
void insert_pte(pde_t *pd, void *addr, void *frame, unsigned int flags);


#endif /* __PG_TABLE_H__ */

