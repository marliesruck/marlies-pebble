/** @file mode_switch.S
 *
 *  @brief Implements a basic kernel-to-user mode switch.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <x86/seg.h>
#include <x86/eflags.h>


#define EFL_USER_MODE EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF & ~EFL_AC

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


