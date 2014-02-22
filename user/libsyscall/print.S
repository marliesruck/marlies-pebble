/** @file print.S
 *
 *  @brief Implements the print(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the print(...) system call.
 **/
SYSCALLn print,$PRINT_INT

