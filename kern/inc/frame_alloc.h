/** @file frame_alloc.h
 *
 *  @brief Delcares the frame allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __FRAME_ALLOC_H__
#define __FRAME_ALLOC_H__


void *alloc_frame(void);
void free_frame(void *frame);
int frame_remaining(void);


#endif /* __FRAME_ALLOC_H__ */

