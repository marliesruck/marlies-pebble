/** @file tlb.h
 *
 *  @brief Implements TLB-related functionality.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs.
 */

#ifndef __TLB_H__
#define __TLB_H__

#include <page_alloc.h>

void tlb_inval_page(void *pg);
void tlb_inval_tome(void *pt);
void tlb_inval_pde(pg_info_s *pgi, void *vaddr);


#endif /* __TLB_H__ */
