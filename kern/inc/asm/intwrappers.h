/** @files intwrappers.h
 *
 *  @brief Contains generic assembly wrapper macros for interrupts.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 **/
#ifndef __INTWRAPPERS_H__
#define __INTWRAPPERS_H__


#ifdef ASSEMBLER

/** @brief Wraps an interrupt handler.
 *
 *  All the interrupts need the same assembly wrapper; we leverage that
 *  with this generic assembly macros.
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Whatever the handler returns.
 **/
.macro VOID_INTERRUPT handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:
  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  # Epilogue
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the interrupt

.endm

/** @brief Wraps a fault handler with an error code that cannot have a handler
 *  installed by the user.
 *
 *  The pebbles spec does not provide a SWEXN_CAUSE macro for certain faults,
 *  consequently the kernel cannot populate the ureg_t cause field correctly.
 *  As a result, for these faults we do not call the user handler, even if one
 *  is installed. 
 * 
 *  @param handler The name of the kernel handler for this interrupt.
 *
 *  @return Void.
 **/
.macro VOID_FAULT_ERROR handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  add  $4, %esp                   # Readjust ESP to account for the error code 

  # Epilogue
  pop   %ebp                      # Restore old EBP

  iret                            # Return from the interrupt

.endm

/** @brief Wraps a fault handler that has no error code and cannot have a
 *  handler installed.
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Void.
 **/
.macro VOID_FAULT handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  # Epilogue
  pop   %ebp                      # Restore old EBP

  iret                            # Return from the interrupt

.endm


/** @brief Wraps a fault handler with an error code that may have a user handler
 *   installed.
 *
 *  This routine passes the address of the kernel handler to a generic wrapper
 *  function, which first allow the kernel to attempt to silently handle the
 *  fault, then calls the user defined software exception handler (if
 *  installed), and if all else fails,then proceeds to slaughter the faulting
 *  thread.
 *
 *  Note that we do not have a prologue and epilogue because we cast the
 *  contents of the stack to a ureg_t.
 *
 *  @param handler The address of the kernel handler.
 *
 *  @return Void.
 **/
.macro VOID_FAULT_ERROR_SWEXN handler

# Export and label it...
.extern fault_wrapper
.extern \handler
.global asm_\handler
asm_\handler:

  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  leal \handler, %eax             # Load and push the address of the 
  push %eax                       # kernel interrupt handler
  call fault_wrapper
  add  $4, %esp                   # Clean up the stack

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  add  $4, %esp                   # Readjust ESP to account for the error code 
  iret                            # Return from the interrupt

.endm

/** @brief Wraps a fault handler with an error code that may also have user
 *  defined handler.
 *
 *  This routine passes the address of the kernel handler to a generic wrapper
 *  function, which first allow the kernel to attempt to silently handle the
 *  fault, then calls the user defined software exception handler (if
 *  installed), and if all else fails, proceeds to slaughter the faulting
 *  thread.
 *
 *  Note that we do not have a prologue and epilogue because we cast the
 *  contents of the stack to a ureg_t.
 *
 *  @param handler The address of the kernel handler.
 *
 *  @return Void.
 **/
.macro VOID_FAULT_SWEXN handler

# Export and label it...
.extern fault_wrapper
.extern \handler
.global asm_\handler
asm_\handler:

  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  leal \handler, %eax             # Load and push the address of the 
  push %eax                       # kernel interrupt handler
  call fault_wrapper
  add  $4, %esp                   # Clean up the stack

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  iret                            # Return from the interrupt

.endm

#endif /* ASSEMBLER */


#endif /* __INTWRAPPERS_H__ */

