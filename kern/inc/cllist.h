/** @file cllist.h
 *
 *  @brief Declares the circularly-linked list (cll) API.
 *
 *  Note: This implementation was inspired by the implementation used in
 *  Linux (see /include/linux/list.h in the Linux source tree).
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/
#ifndef __CLLIST_H__
#define __CLLIST_H__

#include <stddef.h>


/** @struct cllist_node
 *  @brief Circularly-linked list node.
 **/
struct cllist_node {
    struct cllist_node *prev;   /**< Pointer to the previous node. **/
    struct cllist_node *next;   /**< Pointer to the next node. **/
    void *data;                 /**< The data this node carries. **/
};
typedef struct cllist_node cll_node;

/** @brief Circularly-linked list.
 *
 *  A list is just a dummy node.  It's previous pointer is the tail of the
 *  list, and it's next pointer the head.
 **/
typedef struct cllist_node cll_list;


/** @brief Statically initialize a cll list.
 *
 *  We point the list's previous and next pointers at the list.  That is,
 *  the dummy node initially points to itself.
 *
 *  @param name The name you've given the list variable.
 *
 *  @return A cll list.
 **/
#define CLL_LIST_INITIALIZER(name) {  \
  .prev = &name,                      \
  .next = &name,                      \
  .data = NULL                        \
}

/** @brief Determine if the a cll list is empty.
 *
 *  @param l The list to check for emptiness.
 *
 *  @return True if the list is empty, false otherwise.
 **/
#define cll_empty(l) \
  ( (l)->prev == (l) && (l)->next == (l) )

/** @brief Extract the data from a cll node.
 *
 *  @param n The node whose data we'll extract.
 *  @param tp The type of the node's data.
 *
 *  @return The node's data, typecast to type tp.
 **/
#define cll_entry(tp,n) \
  ( (tp) (n)->data )

/** @brief Basic list iterator.
 *
 *  @param l The list over which we'll iterate.
 *  @param n A node pointer to use as a cursor.
 *
 *  @return Much convenience.
 **/
#define cll_foreach(l,n) \
  for ((n) = (l)->next; (n) != (l); (n) = (n)->next)


/* Basic operations */
void cll_init_list(cll_list *l);
void cll_init_node(cll_node *n, void *data);
void cll_final_node(cll_node *n);
void cll_insert(cll_node *before, cll_node *at);
cll_node *cll_extract(cll_list *l, cll_node *victim);


/* Debugging */
int cll_check(cll_list *l);


#endif /* __CL_LIST_H__ */

