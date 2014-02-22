/** @file set_cursor_pos.S
 *
 *  @brief Implements the set_cursor_pos(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the set_cursor_pos(...) system call.
 **/
SYSCALLn set_cursor_pos,$SET_CURSOR_POS_INT

