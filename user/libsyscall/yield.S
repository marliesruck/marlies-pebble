/** @file yield.S
 *
 *  @brief Implements the yield(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the yield(...) system call.
 **/
SYSCALL1 yield,$YIELD_INT

