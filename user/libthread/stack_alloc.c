/** @file stack_alloc.c
 *  
 *  @brief Implements a simple stack allocator.
 *
 *  The primary purpose of this module is to allow us to reuse stacks.  To
 *  this end, we maintain a list of free stacks which we can give to new
 *  threads.
 *  
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/

#include <stack.h>

#include <assert.h>
#include <cllist.h>
#include <malloc.h>
#include <syscall.h>
#include <thr_internals.h>

cll_list stack_list = CLL_LIST_INITIALIZER(stack_list);
mutex_t salloc_lock = MUTEX_INITIALIZER(salloc_lock);

unsigned int sp_index = 1;

/** @brief Allocate a new stack.
 *
 *  By new stack, we mean one that is not in the free list (and therefore
 *  has never been used before).
 *
 *  @return A pointer to the base of the new stack.
 */
void *alloc_new_stack(void)
{
  void *base;

  /* Calculate a new thread base address */
  base = (void *)((unsigned) (sp_low) - thread_stack_size * sp_index);

  /* Fail if new_pages(...) fails */
	if (new_pages(base,thread_stack_size))
    return NULL;

  /* Increment sp_index for the next guy */
  ++sp_index;
  return base;
}

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
	if (new_pages(base,thread_stack_size))
    return NULL;

  free(n);
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

  /* Try the free list first */
  base = alloc_from_list();

  /* Use a new stack if that fails */
  if (!base) base = alloc_new_stack();

  mutex_unlock(&salloc_lock);
  return base;
}

/** @brief Create a dummy entry in the free list.
 *
 *  The dummy node contain's no data, but we return a pointer to it's data
 *  field.  The expectation is that the caller will fill in the data field
 *  with his/her stack AFTER they free it.
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

