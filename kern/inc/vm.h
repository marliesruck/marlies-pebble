/** @file vm.h
 *
 *  @brief Delcares the virtual memory allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __VM_H__
#define __VM_H__

#include <cllist.h>
#include <page_alloc.h>


/* Memory region attribute flags */
#define VM_ATTR_RDWR  0x001    /* 0: read/execute; 1: read/write */
#define VM_ATTR_USER  0x002    /* 0: priviledged; 1: user-accessible */

/** @struct mem_region
 *  @brief A contiguous region in memory.
 **/
struct mem_region {
  void *start;          /**< The first byte in the region **/
  void *limit;          /**< The last byte in the region **/
  unsigned int attrs;   /**< Attributes for the region **/
};
typedef struct mem_region mem_region_s;

/** @struct vm_info
 *  @brief Information used by the VM sub-system.
 **/
struct vm_info {
  pg_info_s pg_info;  /**< Information for the page allocator **/
  cll_list mmap;      /**< A list of currently allocated regions **/
};
typedef struct vm_info vm_info_s;

void *vm_alloc(vm_info_s *vmi, void *va_start, size_t len,
               unsigned int attrs);
void vm_free(pte_s *pd, void *addr);


#endif /* __VM_H__ */

