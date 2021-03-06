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
#define VM_ATTR_ZFOD  0x200    /* 0: page is note ZFOD; 1: page is ZFOD */
#define VM_ATTR_NEWPG  0x004

/** @struct vm_info
 *  @brief Information used by the VM sub-system.
 **/
struct vm_info {
  pg_info_s pg_info;  /**< Information for the page allocator **/
  cll_list mmap;      /**< A list of currently allocated regions **/
};
typedef struct vm_info vm_info_s;

void vm_init_allocator(void);
void vm_init(vm_info_s *vmi);
void *vm_alloc(vm_info_s *vmi, void *va_start, size_t len,
               unsigned int attrs);
void vm_free(vm_info_s *vmi, void *va_start);
int vm_set_attrs(vm_info_s *vmi, void *va_start, unsigned int attrs);
int vm_get_attrs(vm_info_s *vmi, void *va_start, unsigned int *dst);
int vm_copy(vm_info_s *dst, vm_info_s *src);
void vm_final(vm_info_s *vmi);
void *vm_find(vm_info_s *vmi, void *addr);


#endif /* __VM_H__ */

