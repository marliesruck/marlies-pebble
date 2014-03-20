/** @file frame_alloc.h
 *
 *  @brief Delcares the frame allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __FRAME_ALLOC_H__
#define __FRAME_ALLOC_H__

int frame_remaining(void);

/* Frame allocator initalization routine */
void init_frame_allocator(void);

/* Frame allocator manipulation routines */
void *retrieve_head(void);
void update_head(void *frame);
void update_head_wrapper(void *vaddr);


#endif /* __FRAME_ALLOC_H__ */

