/** @file set_status.S
 *
 *  @brief Implements the set_status(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the set_status(...) system call.
 **/
SYSCALL1 set_status,$SET_STATUS_INT

