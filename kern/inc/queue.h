/** @file queue.h
 *
 *  @brief Implements queues on top of our linked lists.
 *
 *  This API is essentially just a convenient wrapper for our linked lists.
 *  Since this API is so simple, we've implemented it entirely using
 *  macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <cllist.h>


/** @brief A queue.
 **/
typedef cll_list queue_s;

/** @brief A queue node.
 **/
typedef cll_node queue_node_s;

/** @brief Statically initialize a queue.
 *
 *  @param The name you've given the queue variable.
 *
 *  @return A queue.
 **/
#define QUEUE_INITIALIZER(name)   \
  CLL_LIST_INITIALIZER(name)

/** @brief Initialize a queue.
 *
 *  @param q A pointer to the queue to initialize.
 *
 *  @return Void.
 **/
#define queue_init(q)   \
  cll_init_list(q)

/** @brief Initialize a queue node.
 *
 *  @param q A pointer to the queue node to initialize.
 *  @param e The element to enqueue.
 *
 *  @return Void.
 **/
#define queue_init_node(n,e)   \
  cll_init_node(n,e)

/** @brief Enqueue a queue node.
 *
 *  @param q A pointer to the queue in which to enqueue the new element.
 *  @param n A pointer to queue node struct to use.
 *
 *  @return Void.
 **/
#define queue_enqueue(q,n)  \
  cll_insert(q, n)

/** @brief Dequeue an element.
 *
 *  @param q A pointer to the queue from which to dequeue an element.
 *
 *  @return void
 **/
#define queue_dequeue(q)  \
  cll_extract((q), (q)->next)

/** @brief Determine if the a queue is empty.
 *
 *  @param q The queue to check for emptiness.
 *
 *  @return True if the queue is empty, false otherwise.
 **/
#define queue_empty(q)  \
  cll_empty(q)

/** @brief Extracts the data from a queue node.
 *
 *  @param n The node whose data we'll extract.
 *  @param tp The type of the node's data.
 *
 *  @return The node's data, typecast to type tp.
 **/
#define queue_entry(tp,n)   \
  cll_entry(tp,n)


#endif /* __QUEUE_H__ */

