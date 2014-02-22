/** @files scwrappers.h
 *
 *  @brief Contains generic system call-wrapping assembly macros.
 *
 *  Most of the system call wrappers are exactly the same; these macros are
 *  supposed to take advantage of that.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/
 #ifndef __SCWRAPPERS_H__
 #define __SCWRAPPERS_H__


/** @brief Wraps a zero-argument system call.
 *
 *  @param scname The name of the system call.
 *  @param scnum The system call's interrupt number.
 *
 *  @return Whatever the system call returns.
 **/
.macro SYSCALL0 scname,scnum

# Export and label it...
.global \scname
\scname:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %esi                      # Callee-save :(

  # Do the system call
  int   \scnum                    # Invoke scnum (no arguments to handle)

  # Epilogue
  pop   %esi                      # Restore ESI
  pop   %ebp                      # Restore old EBP
  ret                             # Return to caller

.endm


/** @brief Wraps one-(small-)argument system calls.
 *
 *  @param scname The name of the system call.
 *  @param scnum The system call's interrupt number.
 *
 *  @return Whatever the system call returns.
 **/
.macro SYSCALL1 scname,scnum

# Export and label it...
.global \scname
\scname:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %esi                      # Callee-save :(

  # Do the system call
  movl  0x8(%ebp), %esi           # Move the arg directly into ESI
  int   \scnum                    # Invoke scnum

  # Epilogue
  pop   %esi                      # Restore ESI
  pop   %ebp                      # Restore old EBP
  ret                             # Return to caller

.endm


/** @brief Wraps n-argument and large-arguemnt system calls.
 *
 *  @param scname The name of the system call.
 *  @param scnum The system call's interrupt number.
 *
 *  @return Whatever the system call returns.
 **/
.macro SYSCALLn scname,scnum

# Export and label it...
.global \scname
\scname:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %esi                      # Callee-save :(

  # Do the system call
  leal  0x8(%ebp), %esi           # Load the addr of the first arg into ESI
  int   \scnum                    # Invoke scnum

  # Epilogue
  pop   %esi                      # Restore ESI
  pop   %ebp                      # Restore old EBP
  ret                             # Return to caller

.endm

#endif /* __SCWRAPPERS_H__ */

