/** @file cr_util.h
 *
 *  @brief Declares x86 specific prototypes for paging and protections.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#ifndef __CR_UTIL_H__
#define __CR_UTIL_H__

int interrupts_enabled(void);
void enable_write_protect(void);
void enable_paging(void);
void disable_paging(void);

#endif /* __CR_UTIL_H__ */
