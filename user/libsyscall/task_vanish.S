/** @file task_vanish.S
 *
 *  @brief Implements the task_vanish(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the task_vanish(...) system call.
 **/
SYSCALL1 task_vanish,$TASK_VANISH_INT

