/** @file halt.S
 *
 *  @brief Implements the halt(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the halt(...) system call.
 **/
SYSCALL0 halt,$HALT_INT

