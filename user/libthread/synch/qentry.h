/** @file qentry.h
 *
 *  @brief Defines the queue entry struct used by locking objects for their
 *  wait queues.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */

#ifndef __QENTRY_H__
#define __QENTRY_H__


/** @brief Wait-queue node.
 **/
struct queue_entry {
  int tid;
  int reject;
};
typedef struct queue_entry qentry_s;


#endif /* __QENTRY_H__ */
