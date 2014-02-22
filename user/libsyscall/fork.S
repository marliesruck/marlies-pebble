/** @file fork.S
 *
 *  @brief Implements the fork(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the fork(...) system call.
 **/
SYSCALL0 fork,$FORK_INT

