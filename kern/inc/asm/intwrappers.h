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

/** @brief Wraps a void interrupt handler.
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

  pusha                           # Store GP registers

  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  call \handler                   # Call the interrupt handler

  pop %ds                         # Restore DS data segment
  pop %es                         # Restore ES data segment
  pop %fs                         # Restore FS data segment
  pop %gs                         # Restore GS data segment

  popa                            # Restore GP registers

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

  pop %ds                         # Restore DS data segment
  pop %es                         # Restore ES data segment
  pop %fs                         # Restore FS data segment
  pop %gs                         # Restore GS data segment

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

  pop %ds                         # Restore DS data segment
  pop %es                         # Restore ES data segment
  pop %fs                         # Restore FS data segment
  pop %gs                         # Restore GS data segment

  # Epilogue
  pop   %ebp                      # Restore old EBP
  iret                            # Return from the interrupt

.endm

#endif /* ASSEMBLER */


#endif /* __INTWRAPPERS_H__ */

