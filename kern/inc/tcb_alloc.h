/** @file tcb_alloc.h
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */
#ifndef __TCB_ALLOC_H__
#define __TCB_ALLOC_H__


/* The stack allocator API */
void *stack_alloc(void);
void **stack_create_entry(void);
void stack_populate_entry(void **datap, void *data);


#endif /* __TCB_ALLOC_H__ */

