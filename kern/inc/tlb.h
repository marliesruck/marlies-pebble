/** @file tlb.c
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


void tlb_inval_page(void *pg);
void tlb_inval_tome(void *pt);


#endif /* __TLB_H__ */
