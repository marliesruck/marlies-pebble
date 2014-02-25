/** @file misbehave.S
 *
 *  @brief Implements the misbehave(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the misbehave(...) system call.
 **/
SYSCALL1 misbehave,$MISBEHAVE_INT

