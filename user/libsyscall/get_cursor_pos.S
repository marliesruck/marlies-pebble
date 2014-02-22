/** @file get_cursor_pos.S
 *
 *  @brief Implements the get_cursor_pos(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the get_cursor_pos(...) system call.
 **/
SYSCALLn get_cursor_pos,$GET_CURSOR_POS_INT

