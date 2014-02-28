/** @file stack.h
 *
 *  @brief Contains various thread-related declarations, including the
 *  thread allocator's API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */
#ifndef __STACK_H__
#define __STACK_H__

#include <syscall.h>

#define PAGE_MASK 0xFFFFF000


/* typedef for readability of casting during stack manipulation */
typedef unsigned stack_t;
typedef unsigned esp_t;
typedef unsigned word_t;

/** @brief Calculates the byte immediately above top of a thread stack.
 *  This address is NOT mapped. 
 *
 *  @param base The base of the thread stack.
 *
 *  @return The first byte above the stack.
 **/
#define STACK_HIGH(base)   \
  (void *)((stack_t)(base) + thread_stack_size)

/** @brief Decrements a esp_t.
 *
 *  @param sp The esp_t to decrement.
 **/
#define DECREMENT(sp)   \
  ( sp = (void *)((esp_t*)(sp) - 1) )

/** @brief Emulates the push instruction: decrement then store.
 *
 *  @param sp A pointer to the address to decrement then write to.
 *  @param elem The value to store.
 **/
#define PUSH(sp,elem)                 \
  do {                                \
     DECREMENT(sp);                   \
    *(esp_t*)(sp) = (word_t)(elem);   \
  } while (0)

#define PAGE_ALIGN(x)   \
  (x & PAGE_MASK)

#define PAGE_CEILING(x)   \
((x == 0) ? PAGE_SIZE : ((x + PAGE_SIZE - 1) & PAGE_MASK))

#define EXN_STACK_SIZE PAGE_SIZE

/* The exception handler's stack */
char exn_stack[EXN_STACK_SIZE];

/* The root thread's high/low stack addresses */
void *sp_high;
void *sp_low;

/* Index used for calculating NEW stack base addresses
 * (statically initialized in the stack allocator)
 */
extern unsigned int sp_index;

/* The stack allocator API */
void *stack_alloc(void);
void **stack_create_entry(void);

/* Retrieve ESP */
esp_t get_esp(void);


#endif /* __STACK_H__ */

