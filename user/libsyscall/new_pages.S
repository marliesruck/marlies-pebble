/** @file new_pages.S
 *
 *  @brief Implements the new_pages(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the new_pages(...) system call.
 **/
SYSCALLn new_pages,$NEW_PAGES_INT

