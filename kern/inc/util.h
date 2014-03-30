/** @file util.h
 *
 *  @brief Declares (implements) various utility functions (macros).
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __UTIL_H__
#define __UTIL_H__


#define CEILING(val,align)  \
  ( ((unsigned int) (val) + (align) - 1) & ~((align) - 1))

#define FLOOR(val,align)  \
  ( ((unsigned int) (val)) & ~((align) - 1))

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


#endif /* __UTIL_H__ */

