/** @file readfile.S
 *
 *  @brief Implements the readfile(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the readfile(...) system call.
 **/
SYSCALLn readfile,$READFILE_INT

