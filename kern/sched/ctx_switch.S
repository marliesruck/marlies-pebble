/** @files asm_ctx_switch.S
 *
 *  @brief Assembly routines for context and mode switches.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug We could pass the address of the prev and next tcb if we want to reduce
 *  the number of args we are passing to asm_ctx_switch
 **/

#include <x86/seg.h>
#include <x86/eflags.h>

#define EFL_USER_MODE EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF & ~EFL_AC

.extern set_esp0

.global ctx_switch
ctx_switch:
  # Prologue
  push    %ebp
  mov     %esp, %ebp

  # Save CPU state
  pusha                       # Store GP
  push    %ebp                # Store EBP
  pushfl                      # Store EFLAGS
  
  # Set next thread's esp0
  mov     0x1c(%ebp), %ebx    # Next thread's kernel stack
  push    %ebx
  call    set_esp0
  pop     %ebx

  # Liberally clobber all regs since we already stored them all out
  mov     8(%ebp), %eax       # &prev_sp
  mov     0xc(%ebp), %ecx     # &prev_pc
  mov     0x10(%ebp), %edx    # next_sp
  mov     0x14(%ebp), %esi    # next_pc
  mov     0x18(%ebp), %edi    # next_cr3

  call store_and_switch

  # Restore CPU state
  popfl                       # Restore GP
  pop     %ebp                # Restore EBP
  popa                        # Restore EFLAGS

  # Epilogue
  pop     %ebp
  ret


store_and_switch:
  # Prologue
  push    %ebp
  mov     %esp, %ebp

  # Store out current thread's context
  leal    8(%ebp), %ebx        # Compute old ESP
  mov     %ebx, (%eax)         # Store old ESP in &prev_sp 
  mov     4(%ebp), %ebx        # Retrieve return address from stack
  mov     %ebx, (%ecx)         # Store RA in &prev_pc

  # Load new thread's context
  mov     %edx, %esp           # Load ESP
  mov     %edi, %cr3           # Load correct PTBR
  jmp     *%esi                # Jump to new thread's EIP

.global mode_switch
mode_switch:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  # Load args
  movl  0x8(%ebp), %ecx           # Move user-mode EIP into ECX
  movl  0xC(%ebp), %edx           # Move user-mode ESP into EBX

  # Setup various data segment selectors
  movw  $SEGSEL_USER_DS, %ax      # Can't move into segment regs directly
  movw  %ax, %ds                  # Set DS data segment
  movw  %ax, %es                  # Set ES data segment
  movw  %ax, %fs                  # Set FS data segment
  movw  %ax, %gs                  # Set GS data segment

  # Setup stack for iret
  pushl $SEGSEL_USER_DS           # Push user stack segment selector
  pushl %edx                      # Push user stack pointer
  pushl $EFL_USER_MODE            # Push flag register
  pushl $SEGSEL_USER_CS           # Push user code segment selector
  pushl %ecx                      # Push user program counter

  # Set register values
  movl  %edx, %ebp                # EBP = user ESP
  xorl  %eax, %eax                # Clear EAX
  xorl  %ebx, %ebx                # Clear EBX
  xorl  %ecx, %ecx                # Clear ECX
  xorl  %edx, %edx                # Clear EDX
  xorl  %esi, %esi                # Clear ESI
  xorl  %edi, %edi                # Clear EBI

  # Perform the IRET
  iret                            # To user-space!

  # We shouldn't return from IRET

