/** @file vm.h
 *
 *  @brief Delcares the virtual memory allocator API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __VM_H__
#define __VM_H__

#include <pg_table.h>


void *vm_alloc(pte_t *pd, void *addr, size_t len, unsigned int attrs);
void vm_free(pte_t *pd, void *addr);


#endif /* __VM_H__ */

