/** @file remove_pages.S
 *
 *  @brief Implements the remove_pages(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the remove_pages(...) system call.
 **/
SYSCALL1 remove_pages,$REMOVE_PAGES_INT

