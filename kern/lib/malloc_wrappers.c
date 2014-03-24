#include <stddef.h>
#include <malloc.h>
#include <malloc/malloc_internal.h>
#include <mutex.h>

static mutex_s allocator_lock = MUTEX_INITIALIZER(allocator_lock);

/* safe versions of malloc functions */
void *malloc(size_t size)
{
  mutex_lock(&allocator_lock);
  void *addr = _malloc(size); 
  mutex_unlock(&allocator_lock);
  return addr;
}

void *memalign(size_t alignment, size_t size)
{
  mutex_lock(&allocator_lock);
  void *addr = _memalign(alignment,size);
  mutex_unlock(&allocator_lock);
  return addr;
}

void *calloc(size_t nelt, size_t eltsize)
{
  mutex_lock(&allocator_lock);
  void *addr = _calloc(nelt,eltsize);
  mutex_unlock(&allocator_lock);
  return addr;
}

void *realloc(void *buf, size_t new_size)
{
  mutex_lock(&allocator_lock);
  void *addr = _realloc(buf,new_size);
  mutex_unlock(&allocator_lock);
  return addr;
}

void free(void *buf)
{
  mutex_lock(&allocator_lock);
  _free(buf);
  mutex_unlock(&allocator_lock);
  return;
}

void *smalloc(size_t size)
{
  mutex_lock(&allocator_lock);
  void *addr = _smalloc(size);
  mutex_unlock(&allocator_lock);
  return addr;
}

void *smemalign(size_t alignment, size_t size)
{
  mutex_lock(&allocator_lock);
  void *addr = _smemalign(alignment,size);
  mutex_unlock(&allocator_lock);
  return addr;
}

void sfree(void *buf, size_t size)
{
  mutex_lock(&allocator_lock);
  _sfree(buf,size);
  mutex_unlock(&allocator_lock);
  return;
}


