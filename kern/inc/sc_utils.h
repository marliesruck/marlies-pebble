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


#ifndef ASSEMBLER

/* Pebbles specific includes */
#include <types.h>

/* Libc specific includes */
#include <ureg.h>
#include <stdlib.h>

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);

/** @brief Each thread may optionall register with the kernel a software
 *         exception handler.
 **/
typedef struct swexn {
  void *esp3;
  swexn_handler_t eip;
  void *arg;
} swexn_t;

void deregister(swexn_t *swexn);
int validate_regs(ureg_t *regs);

int copy_from_user(char *dst, const char *src, size_t bytes);
void install_sys_handlers(void);
int sc_validate_argp(void *argp, int arity);

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

  add  $4, %esp                   # Clean up stack

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
  push  %ebp                      # Store USER EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %ds                       # Store DS data segment
  push  %es                       # Store ES data segment
  push  %fs                       # Store FS data segment
  push  %gs                       # Store GS data segment

  # Store MOVS registers
  push  %esi                      # Save ESI
  push  %edi                      # Save EDI
  push  %ebx                      # Save EBX

  movl \argc, %ecx                # ECX = argument count (for rep)
  leal (,%ecx, 4), %ebx           # Compute space on stack for args
  sub  %ebx, %esp                 # Make space on the stack for the args
  movl %esp, %edi                 # EDI = kernel stack
  rep movsl                       # Copy args onto kernel stack

  # Invoke system call handler
  call \scname                    # Call the system call handler
  add   %ebx, %esp                # Clean up ESP from args pushed on by rep

  # Restore MOVS registers
  pop   %ebx                      # Restore EBX
  pop   %edi                      # Restore EDI
  pop   %esi                      # Restore ESI

  # Epilogue
  pop   %gs                       # Restore GS data segment
  pop   %fs                       # Restore FS data segment
  pop   %es                       # Restore ES data segment
  pop   %ds                       # Restore DS data segment
  pop   %ebp                      # Restore USER EBP
  iret                            # Return from the system call

.endm


/** @brief Wraps the fork and thread fork system call handlers.
 *
 *  We need a special wrapper for these two because the C wrappers need to
 *  know the value of ESP.
 *
 *  @param scname The name of the system call.
 *
 *  @return Whatever the system call returns.
 **/
.macro FORK_SYSCALL scname

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

  # Save callee-saved registers for the child
  push  %esi
  push  %edi
  push  %ebx

  # Pass ESP and invoke the handler
  push  %esp
  call  \scname

  # Parent starts here
  add  $4, %esp                 # Clean up stack
  jmp   asm_finish_\scname      # Jump to cleanup

  # Child starts here
  .global asm_child_finish_\scname
  asm_child_finish_\scname:
    xor %eax,%eax                 # Child returns 0

  asm_finish_\scname:
    # Restore callee-saved registers
    pop   %ebx
    pop   %edi
    pop   %esi
    
    # Epilogue
    pop   %gs                     # Restore GS data segment
    pop   %fs                     # Restore FS data segment
    pop   %es                     # Restore ES data segment
    pop   %ds                     # Restore DS data segment
    pop   %ebp                    # Restore USER EBP
    iret                          # Return from the system call

.endm


#endif /* ASSEMBLER */


#endif /* __SC_UTILS_H__ */


