/** @files intwrappers.h
 *
 *  @brief Contains generic syscall-wrapping assembly macros.
 *
 *  All the system call wrappers are pretty similar; we leverage that with
 *  these generic assembly macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/
#ifndef __SC_UTILS_H__
#define __SC_UTILS_H__


/* System call arity constants */
#define SC_NULLARY  0
#define SC_UNARY    1
#define SC_N_ARY    2


#ifndef ASSEMBLER

#include <types.h>


int copy_from_user(char *dst, const char *src, size_t bytes);

#else /* ASSEMBLER */

.extern sim_breakpoint

/** @brief Wraps a nullary system call handler.
 *
 *  @param scname The name of the system call.
 *
 *  @return Whatever the system call returns.
 **/
.macro NULLARY_SYSCALL scname

# Export and label it...
.extern \scname
.global asm_\scname
asm_\scname:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %ds                       # Store DS data segment
  push  %es                       # Store ES data segment
  push  %fs                       # Store FS data segment
  push  %gs                       # Store GS data segment
  
  # Invoke system call handler
  call \scname                    # Call the system call handler

  # Epilogue
  pop   %gs                       # Restore GS data segment
  pop   %fs                       # Restore FS data segment
  pop   %es                       # Restore ES data segment
  pop   %ds                       # Restore DS data segment
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the system call

.endm


/** @brief Wraps a unary system call handler.
 *
 *  @param scname The name of the system call.
 *
 *  @return Whatever the system call returns.
 **/ .macro UNARY_SYSCALL scname

# Export and label it...
.extern \scname
.global asm_\scname
asm_\scname:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %ds                       # Store DS data segment
  push  %es                       # Store ES data segment
  push  %fs                       # Store FS data segment
  push  %gs                       # Store GS data segment
  
  # Invoke system call handler
  push %esi                       # Push single argument
  call \scname                    # Call the system call handler

  # Epilogue
  pop   %gs                       # Restore GS data segment
  pop   %fs                       # Restore FS data segment
  pop   %es                       # Restore ES data segment
  pop   %ds                       # Restore DS data segment
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the system call

.endm


/** @brief Wraps a n-ary system call handler.
 *
 *  TODO: We need to validate the parameters, no just blindly copy them.
 *
 *  @param scname The name of the system call.
 *
 *  @return Whatever the system call returns.
 **/
.macro N_ARY_SYSCALL scname,argc

# Export and label it...
.extern \scname
.global asm_\scname
asm_\scname:

  # Prologue
  call sim_breakpoint
  push  %ebp                      # Store USER EBP
  movl  %esp, %ebp                # Set up new EBP

  push  %edi                      # Save EDI

  push  %ds                       # Store DS data segment
  push  %es                       # Store ES data segment
  push  %fs                       # Store FS data segment
  push  %gs                       # Store GS data segment
  
  # Invoke system call handler
  movl \argc, %ecx                # ECX = argument count (for rep)
  movl %esp, %edi                 # EDI = kernel stack
  rep movsl                       # Copy args onto kernel stack

  call sim_breakpoint

  call \scname                    # Call the system call handler

  call sim_breakpoint

  pop   %ecx
  pop   %edi
# leal  -8(%esp), %esp            # Clean up ESP

  # Epilogue
  pop   %gs                       # Restore GS data segment
  pop   %fs                       # Restore FS data segment
  pop   %es                       # Restore ES data segment
  pop   %ds                       # Restore DS data segment

  pop   %edi                      # Restore EDI

  pop   %ebp                      # Restore USER EBP

  iret                            # Return from the system call

.endm

#endif /* ASSEMBLER */


#endif /* __SC_UTILS_H__ */

