/** @file getchar.S
 *
 *  @brief Implements the getchar(...) system call's assembly wrapper.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <syscall_int.h>
#include <scwrappers.h>


/** @brief Wraps the getchar(...) system call.
 **/
SYSCALL0 getchar,$GETCHAR_INT

