/** @file frame_alloc.h
 *
 *  @brief Delcares the page allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PAGE_ALLOC_H__
#define __PAGE_ALLOC_H__


void alloc_page(pte_s *pd, void *vaddr, unsigned int attrs);
int page_is_allocated(pte_s *pd, void *vaddr);


#endif /* __PAGE_ALLOC_H__ */

