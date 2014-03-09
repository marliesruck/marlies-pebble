/** @file cllist.c
 *
 *  @brief Implements a generic circularly-linked list.
 *
 *  @author Enrique Naudon (esn)
 **/
#include <simics.h>

#include <cllist.h>

#include <assert.h>
#include <stddef.h>


/* Internal typedef for convenience;
 * externally, prefer those in the header.
 */
typedef struct cllist_node node;
typedef struct cllist_node list;


/*************************************************************************
 *  Internal helpers and debug functions
 *************************************************************************/

/** @brief Sets the previous and next pointers of a node to NULL.
 *
 *  @param n The node to zero.
 *
 *  @return Void.
 **/
inline void zero(node *n)
{
  n->prev = NULL;
  n->next = NULL;
  return;
}

int check_pointers(node *n)
{
  /* A node's predecessor should point at it */
  if (n->prev && n->prev->next != n) return -1;

  /* The same goes for the successor */
  else if (n->next && n->next->prev != n) return -1;

  return 0;
}

int cll_check(list *l)
{
  node *n;

  /* The head's prev/next pointers should never be NULL */
  if (l->prev == NULL) return -1;
  else if (l->next == NULL) return -1;

  /* Make everything points where it should */
  cll_foreach(l, n)
    if (check_pointers(n)) {
      return -1;
    }

  return 0;
}


/*************************************************************************
 *  Basic operations
 *************************************************************************/

/** @brief Initialize a cll list.
 *
 *  @param l The list to initialize.
 *
 *  @return Void.
 **/
void cll_init_list(list *l)
{
  l->prev = l;
  l->next = l;
  l->data = NULL;

  return;
}

/** @brief Initialize a cll node.
 *
 *  Using a node prior to initialization is not recommended.
 *
 *  @param n The node to initialize.
 *  @param data The data the node will carry.
 *
 *  @return Void.
 **/
void cll_init_node(node *n, void *data)
{
  n->prev = NULL;
  n->next = NULL;
  n->data = data;

  return;
}

/** @brief Finalize a cll node.
 *
 *  Nodes should be finalized after removal from a list.
 *
 *  @param n The node to finalize.
 *
 *  @return Void.
 **/
void cll_final_node(node *n)
{
  zero(n);
  return;
}

/** @brief Insert a node into a cll list.
 *
 *  @param l The list into which we insert the new node.
 *  @param before The node before which we insert the new node.
 *  @param new The new node.
 *
 *  @return Void.
 **/
void cll_insert(node *before, node *new)
{
  assert(new);
  assert(before);

  /* Make before's prev and new point to each other */
  before->prev->next = new;
  new->prev = before->prev;

  /* Make before and new point to each other */
  before->prev = new;
  new->next = before;

  return;
}

/** @brief Extract a node from a cll list.
 *
 *  @param l The list from which we extract the node.
 *  @param victim The node to extract.
 *
 *  @return A pointer to the extracted node.
 **/
node *cll_extract(list *l, node *victim)
{
  assert(l);
  assert(victim);
  assert(l != victim);

  victim->next->prev = victim->prev;
  victim->prev->next = victim->next;

  zero(victim);

  return victim;
}

