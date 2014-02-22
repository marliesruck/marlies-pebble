/** @file readline.S
 *
 *  @brief Implements the readline(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the readline(...) system call.
 **/
SYSCALLn readline,$READLINE_INT

