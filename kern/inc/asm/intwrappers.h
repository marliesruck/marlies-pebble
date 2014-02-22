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

/** @brief Wraps an interrupt.
 *
 *  @param handler The name of the handler for this interrupt.
 *
 *  @return Whatever the handler returns.
 **/
.macro INTERRUPT handler

# Export and label it...
.extern \handler
.global asm_\handler
asm_\handler:

  pusha                           # Store GP registers
  call \handler                   # Call the interrupt handler
  popa                            # Restore GP registers
  iret                            # Return from the interrupt

.endm

#endif /* ASSEMBLER */


#endif /* __INTWRAPPERS_H__ */

