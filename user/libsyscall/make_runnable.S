/** @file make_runnable.S
 *
 *  @brief Implements the make_runnable(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the make_runnable(...) system call.
 **/
SYSCALL1 make_runnable,$MAKE_RUNNABLE_INT

