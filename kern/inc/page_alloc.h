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

void pg_init_allocator(void);
void *alloc_page_table(pg_info_s *pgi, void *vaddr);
void *alloc_page(pg_info_s *pgi, void *vaddr, unsigned int attrs);
void *pg_alloc_phys(pg_info_s *pgi, void *vaddr);
void free_page(pg_info_s *pgi, void *vaddr);
int page_set_attrs(pg_info_s *pgi, void *vaddr, unsigned int attrs);
int copy_page(pg_info_s *dst, pg_info_s *src, void *vaddr);


#endif /* __PAGE_ALLOC_H__ */

