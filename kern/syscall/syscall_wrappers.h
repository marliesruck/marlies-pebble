/** @file syscall_wrappers.h
 *
 *  @brief Declares assembly wrapper for the various syscalls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __SYSCALL_WRAPPERS_H__
#define __SYSCALL_WRAPPERS_H__


int asm_sys_gettid(void);
char asm_sys_getchar(void);


#endif /* __SYSCALL_WRAPPERS_H__ */

