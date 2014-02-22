/** @file swexn.S
 *
 *  @brief Implements the swexn(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the swexn(...) system call.
 **/
SYSCALLn swexn,$SWEXN_INT

