/** @file set_term_color.S
 *
 *  @brief Implements the set_term_color(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the set_term_color(...) system call.
 **/
SYSCALL1 set_term_color,$SET_TERM_COLOR_INT

