#ifndef _USR_STACK_H
#define _USR_STACK_H

#include <pg_table.h>
#include <vm.h>
#include <x86/page.h>


#define USR_STACK_SIZE PAGE_SIZE
#define USR_SP_HI (void *)(DIR_HIGH)

typedef unsigned int esp_t;

/** @brief Decrements a esp_t.
 *
 *  @param sp The esp_t to decrement.
 **/
#define DECREMENT(sp)   \
  ( sp = (void *)((esp_t *)(sp) - 1) )

/** @brief Emulates the push instruction: decrement then store.
 *
 *  @param sp A pointer to the address to decrement then write to.
 *  @param elem The value to store.
 **/
#define PUSH(sp,elem)                   \
  do {                                  \
    DECREMENT(sp);                      \
    *(esp_t *)(sp) = (unsigned)(elem);  \
  } while (0)

void *usr_stack_init(vm_info_s *vmi, char **argv);

#endif /* _USR_STACK_H */
