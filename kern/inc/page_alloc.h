/** @file frame_alloc.h
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
  pte_s *pg_dir;    /**< The page directory. **/
  pt_t  *pg_tbls;   /**< The page tables. **/
};
typedef struct page_info pg_info_s;

/* ZFOD dummy frame */
void *zfod;

void *alloc_page(pg_info_s *pgi, void *vaddr, unsigned int attrs);
void free_page(pg_info_s *pgi, void *vaddr);
int copy_page(pg_info_s *dst, const pg_info_s *src, void *vaddr, unsigned int attrs);


#endif /* __PAGE_ALLOC_H__ */

