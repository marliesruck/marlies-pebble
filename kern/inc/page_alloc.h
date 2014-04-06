/** @file page_alloc.h
 *
 *  @brief Delcares the page allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PAGE_ALLOC_H__
#define __PAGE_ALLOC_H__

#include <pg_table.h>


/** @struct page_info
 *  @brief Information used by the page allocator.
 **/
struct page_info {
  pte_t *pg_dir;    /**< The page directory. **/
  pt_t  *pg_tbls;   /**< The page tables. **/
};
typedef struct page_info pg_info_s;

int pg_init_allocator(void);

/* Page stuff */
void *pg_alloc(pg_info_s *pgi, void *vaddr, unsigned int attrs);
void pg_free(pg_info_s *pgi, void *vaddr);
int pg_set_attrs(pg_info_s *pgi, void *vaddr, unsigned int attrs);
int pg_copy(pg_info_s *dst, pg_info_s *src, void *vaddr, void *buf);
int pg_page_fault_handler(void *addr);

/* Page table stuff */
void pg_free_table(pg_info_s *pgi, void *vaddr);

/*** Invariant checkers ***/
void validate_pd(pg_info_s *pgi);
void validate_pt(pg_info_s *pgi, void *vaddr);
  

#endif /* __PAGE_ALLOC_H__ */

