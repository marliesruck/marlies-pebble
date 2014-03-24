/** @file fault_wrappers.S
 *
 *  @brief Implements assembly wrapper for the various faults.
 *
 *  See /kern/asm/intwrappers.h for the expanded *_INTERRUPT macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <asm/intwrappers.h>


VOID_INTERRUPT int_divzero
VOID_INTERRUPT int_debug
VOID_INTERRUPT int_nmi
VOID_INTERRUPT int_breakpoint
VOID_INTERRUPT int_overflow
VOID_INTERRUPT int_bound
VOID_INTERRUPT int_undef_opcode
VOID_INTERRUPT int_device_unavail
VOID_INTERRUPT int_double_fault
VOID_INTERRUPT int_cso
VOID_INTERRUPT int_tss
VOID_INTERRUPT int_seg_not_present
VOID_INTERRUPT int_stack_seg
VOID_INTERRUPT int_gen_prot
VOID_INTERRUPT int_float
VOID_INTERRUPT int_align
VOID_INTERRUPT int_machine_check
VOID_INTERRUPT int_simd
VOID_INTERRUPT int_generic

# VOID_INTERRUPT int_page_fault

.extern sim_breakpoint
.extern int_page_fault
.global asm_int_page_fault

asm_int_page_fault:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  
  pusha                           # Store GP registers
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  mov 52(%esp), %eax              # Grab error code
  push %eax
  mov 56(%esp), %eax              # Grab address of faulting instruction
  push %eax
  
  call int_page_fault             # Call the interrupt handler
  add  $8, %esp                   # Readjust ESP to account for the error code
                                  # push as an argument

  # Epilogue
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment
  popa                            # Restore GP registers
  
  # Epilogue
  pop  %ebp                       # Restore old EBP
  add  $4, %esp                   # Readjust ESP to account for the error code 
                                  # that the processor pushed
     
  iret                            # Return from the interrupt


