/**
 **/

#include "sc_utils.h"

#include <assert.h>
#include <string.h>


/** @brief Safely invoke kernel system call handlers.
 *
 *  After validating the system call arguments, we invoke the handler and
 *  pass the arguments along.  For simplicity, we assume that all system
 *  call handlers return an unsigned 32-bit integer; it is the caller's
 *  responsibility to ignore void values.
 *
 *  @param func The system call handler.
 *  @param args The arguments to the handler.
 *  @param arity The arity of the handler.
 *
 *  @return Whatever the sytem call handler returns.
 **/
int sc_validate_argp(void *argp, int arity)
{
  return 0;
}

/** @brief Safely copy data from user-space.
 *
 *  TODO: This isn't actually safe; make it so.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) source pointer.
 *  @param bytes The number of bytes to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_from_user(char *dst, const char *src, size_t bytes)
{
  memcpy(dst, src, bytes);
  return 0;
}

