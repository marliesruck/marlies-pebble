/** @file cond_type.h
 *
 *  @brief This file defines the type for condition variables.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <spin.h>
#include <queue.h>

typedef struct cond {
	spin_s lock;
	queue_s queue;
} cond_t;

#endif /* _COND_TYPE_H */
