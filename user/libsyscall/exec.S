/** @file exec.S
 *
 *  @brief Implements the exec(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the exec(...) system call.
 **/
SYSCALLn exec,$EXEC_INT

