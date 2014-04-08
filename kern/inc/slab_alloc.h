/** @file tcb_alloc.h
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */
#ifndef __SLAB_ALLOC_H__
#define __SLAB_ALLOC_H__


/* The slab allocator API */
void *slab_alloc(void);
void **slab_create_entry(void);
void slab_populate_entry(void **datap, void *data);


#endif /* __SLAB_ALLOC_H__ */

