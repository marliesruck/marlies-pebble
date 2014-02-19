/** @file pg_table.c
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


#endif /* __FRAME_ALLOC_H__ */

