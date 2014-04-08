/** @file tcb_alloc.c
 *  
 *  @brief Implements a simple SLAB allocator.
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


cll_list slab_list = CLL_LIST_INITIALIZER(slab_list);
mutex_s salloc_lock = MUTEX_INITIALIZER(salloc_lock);

/** @brief Allocate a slab from the free list.
 *
 *  @return A pointer to the base of the slab.
 **/
void *alloc_from_list(void)
{
  cll_node *n;
  void *base;

  /* Search for a usable slab */
  cll_foreach(&slab_list, n)
    if (cll_entry(void *,n)) break;
  if (n == &slab_list) return NULL;

  /* Grab one from the list */
  n = cll_extract(&slab_list, n);
  base = cll_entry(void *, n);

  free(n);
  memset(base, 0, PAGE_SIZE);
  return base;
}

/** @brief Allocate a slab.
 *
 *  @return A pointer to the base of the slab.
 **/
void *slab_alloc(void)
{
  void *base;

  mutex_lock(&salloc_lock);
  base = alloc_from_list();
  mutex_unlock(&salloc_lock);

  /* Use a new slab if that fails */
  if (!base) base = smemalign(PAGE_SIZE, PAGE_SIZE);

  return base;
}

/** @brief Create a dummy entry in the free list.
 *
 *  The dummy node contain's no data, but we return a pointer to it's data
 *  field.  The expectation is that the caller will fill in the data field
 *  with his/her slab.
 *
 *  @return A pointer to the data field of the dummy entry.
 **/
void **slab_create_entry(void)
{
  cll_node *n;

  /* Malloc and init the dummy node */
  n = malloc(sizeof(cll_node));
  if (n == NULL) return NULL;
  cll_init_node(n, NULL);

  /* Add it to the list */
  mutex_lock(&salloc_lock);
  cll_insert(slab_list.next, n);
  mutex_unlock(&salloc_lock);

  return &n->data;
}

/** @brief Populate a dummy entry.
 *
 *  @return Void.
 **/
void slab_populate_entry(void **datap, void *data)
{
  *datap = data;
  return;
}

