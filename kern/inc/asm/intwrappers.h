/** @files intwrappers.h
 *
 *  @brief Contains generic interrupt-wrapping assembly macros.
 *
 *  All the interrupts, traps and interrupts proper, need the same assembly
 *  wrappers.  We leverage that with these generic assembly macros.
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


/** @brief Wraps a interrupt handler which returns a 8-bit value.
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Whatever the handler returns.
 **/
.macro BYTE_INTERRUPT handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  # Epilogue
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the interrupt

.endm


/** @brief Wraps a interrupt handler which returns a 32-bit value.
 *
 *  Note that the "LONG" prefix refers to the 32-bit return value, *not*
 *  the amount of time the handler is expected to take...
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Whatever the handler returns.
 **/
.macro LONG_INTERRUPT handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers

  # Epilogue
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the interrupt

.endm

/** @brief Wraps a fault handler with an error code.
 *
 *  All the fault handlers with error codes need the same assembly wrapper;
 *  we leverage that with this generic assembly macros.
 *
 *  We take advantage of the order registers are pushed on the stack so that we
 *  can simply cast the stack to a ureg for the user exception handler.
 *  Consequently, we do not have a prologue or epilogue.
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Void.
 **/
.macro VOID_FAULT_ERROR handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

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
  iret                            # Return from the interrupt

.endm

/** @brief Wraps a fault handler that has no error code.
 *
 *  All the fault handlers with no error code need the same assembly wrapper;
 *  we leverage that with this generic assembly macros.
 *
 *  We take advantage of the order registers are pushed on the stack so that we
 *  can simply cast the stack to a ureg for the user exception handler.
 *  Consequently, we do not have a prologue or epilogue.
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

  iret                            # Return from the interrupt

.endm


/** @brief Wraps a fault handler with an error code that may also have user
 *  defined handler.
 *
 *  This routine passes the address of the kernel handler to a generic wrapper
 *  function, which first allow the kernel to attempt to silently handle the
 *  fault, then calls the user defined software exception handler (if
 *  installed), and if all else fails, then proceeds to slaughter the faulting thread.
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

  push \handler                   # Push the interrupt handler
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
 *  installed), and if all else fails, then proceeds to slaughter the faulting thread.
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

  push \handler                   # Push the kernel interrupt handler
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

