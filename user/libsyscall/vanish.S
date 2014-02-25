/** @file vanish.S
 *
 *  @brief Implements the vanish(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the vanish(...) system call.
 **/
SYSCALL0 vanish,$VANISH_INT

