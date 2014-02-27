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
#include <pg_table.h>


/** @struct mem_region
 *  @brief A contiguous region in memory.
 **/
struct mem_region {
  void *start;          /**< The first byte in the region. **/
  void *limit;          /**< The last byte in the region. **/
  unsigned int attrs;   /**< Attributes for the region. **/
};
typedef struct mem_region mem_region_s;

/** @struct vm_info
 **/
struct vm_info {
  pte_t *pg_dir;    /**< The page directory. **/
  pt_t  *pg_tbls;   /**< The page tables. **/
  cll_list mmap;    /**< A list of currently allocated regions **/
};
typedef struct vm_info vm_info_s;

void *vm_alloc(pte_t *pd, void *addr, size_t len, unsigned int attrs);
void vm_free(pte_t *pd, void *addr);


#endif /* __VM_H__ */

