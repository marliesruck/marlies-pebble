/** @file sleep.S
 *
 *  @brief Implements the sleep(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the sleep(...) system call.
 **/
SYSCALL1 sleep,$SLEEP_INT

