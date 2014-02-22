/** @file get_ticks.S
 *
 *  @brief Implements the get_ticks(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the get_ticks(...) system call.
 **/
SYSCALL0 get_ticks,$GET_TICKS_INT

