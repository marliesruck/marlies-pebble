/** @file frame_alloc.h
 *
 *  @brief Delcares the frame allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __FRAME_ALLOC_H__
#define __FRAME_ALLOC_H__

#include <mutex.h>


/* Serialize access to the frame allocator */
extern mutex_s frame_allocator_lock;

int frame_remaining(void);

/* Frame allocator initalization routine */
void fr_init_allocator(void);

/* Frame allocator manipulation routines */
void *retrieve_head(void);
void update_head(void *frame);


#endif /* __FRAME_ALLOC_H__ */

