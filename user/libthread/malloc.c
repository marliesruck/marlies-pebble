/* @file malloc.c
 *
 * @brief Thread safe versions of functions in the malloc family.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No knowns bugs
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>

mutex_t malloc_lock = MUTEX_INITIALIZER(malloc_lock);

/** @brief Thread-safe wrapper for malloc.
 *  
 *  @param __size The number of bytes to allocate.
 **/
void *malloc(size_t __size)
{
  mutex_lock(&malloc_lock);  
  void *addr = _malloc(__size);
  mutex_unlock(&malloc_lock);
  return addr;
}

/** @brief Thread-safe wrapper for calloc.
 *  
 *  @param __nelt The number of elements to allocate memory for.
 *  @param __eltsize The size of each element in bytes.
 **/
void *calloc(size_t __nelt, size_t __eltsize)
{
  mutex_lock(&malloc_lock);  
  void *addr = _calloc(__nelt, __eltsize);
  mutex_unlock(&malloc_lock);
  return addr;
}

/** @brief Thread-safe wrapper for realloc.
 *  
 *  @param __buf A pointer to the old allocation.
 *  @param __new_size The desired size of the new allocation.
 **/
void *realloc(void *__buf, size_t __new_size)
{
  mutex_lock(&malloc_lock);  
  void *addr = _realloc(__buf, __new_size);
  mutex_unlock(&malloc_lock);
  return addr;
}

/** @brief Thread-safe wrapper for free.
 *  
 *  @param __buf A pointer to the memory to free.
 **/
void free(void *__buf)
{
  mutex_lock(&malloc_lock);  
  _free(__buf);
  mutex_unlock(&malloc_lock);
  return;
}

