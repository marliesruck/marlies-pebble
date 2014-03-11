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


#endif /* __UTIL_H__ */

