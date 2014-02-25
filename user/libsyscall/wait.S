/** @file wait.S
 *
 *  @brief Implements the wait(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the wait(...) system call.
 **/
SYSCALL1 wait,$WAIT_INT

