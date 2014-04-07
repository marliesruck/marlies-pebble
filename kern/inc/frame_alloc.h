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
#include <x86/page.h>


/* Frame types */
typedef char frame_t[PAGE_SIZE];
extern const frame_t *frames;

/* Serialize access to the frame allocator */
extern mutex_s frame_allocator_lock;

/* Number of available free frames */
extern int fr_avail;

/* Frame allocator API */
void fr_init_allocator(void);
void *fr_retrieve_head(void);
void fr_update_head(void *frame);


#endif /* __FRAME_ALLOC_H__ */

