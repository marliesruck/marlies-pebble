/** @file deschedule.S
 *
 *  @brief Implements the deschedule(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the deschedule(...) system call.
 **/
SYSCALL1 deschedule,$DESCHEDULE_INT

