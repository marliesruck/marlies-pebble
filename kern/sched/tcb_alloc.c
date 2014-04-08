/** @file tcb_alloc.c
 *  
 *  @brief Implements a simple TCB allocator.
 *
 *  The primary purpose of this module is to allow us to reuse TCBs before
 *  the parent gets around to cleaning up after the exited thread.  To
 *  this end, we maintain a list of not-in-use, but not yet freed stacks
 *  which we can give to new threads.
 *  
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/

#include <cllist.h>
#include <mutex.h>
#include <thread.h>

#include <assert.h>
#include <malloc.h>
#include <string.h>


cll_list stack_list = CLL_LIST_INITIALIZER(stack_list);
mutex_s salloc_lock = MUTEX_INITIALIZER(salloc_lock);

/** @brief Allocate a stack from the free list.
 *
 *  @return A pointer to the base of the stack.
 **/
void *alloc_from_list(void)
{
  cll_node *n;
  void *base;

  /* Search for a usable stack */
  cll_foreach(&stack_list, n)
    if (cll_entry(void *,n)) break;
  if (n == &stack_list) return NULL;

  /* Grab one from the list */
  n = cll_extract(&stack_list, n);
  base = cll_entry(void *, n);

  free(n);
  memset(base, 0, sizeof(thread_t));
  return base;
}

/** @brief Allocate a stack.
 *
 *  @return A pointer to the base of the stack.
 **/
void *stack_alloc(void)
{
  void *base;

  mutex_lock(&salloc_lock);
  base = alloc_from_list();
  mutex_unlock(&salloc_lock);

  /* Use a new stack if that fails */
  if (!base) base = malloc(sizeof(thread_t));

  return base;
}

/** @brief Create a dummy entry in the free list.
 *
 *  The dummy node contain's no data, but we return a pointer to it's data
 *  field.  The expectation is that the caller will fill in the data field
 *  with his/her stack.
 *
 *  @return A pointer to the data field of the dummy entry.
 **/
void **stack_create_entry(void)
{
  cll_node *n;

  /* Malloc and init the dummy node */
  n = malloc(sizeof(cll_node));
  if (n == NULL) return NULL;
  cll_init_node(n, NULL);

  /* Add it to the list */
  mutex_lock(&salloc_lock);
  cll_insert(stack_list.next, n);
  mutex_unlock(&salloc_lock);

  return &n->data;
}

/** @brief Populate a dummy entry.
 *
 *  @return Void.
 **/
void stack_populate_entry(void **datap, void *data)
{
  *datap = data;
  return;
}

