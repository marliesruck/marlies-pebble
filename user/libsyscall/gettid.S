/** @file gettid.S
 *
 *  @brief Implements the gettid(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>

/** @brief Wraps the gettid(...) system call.
 **/
SYSCALL0 gettid,$GETTID_INT

