/** @file mode_switch.S
 *
 *  @brief Implements a basic kernel-to-user mode switch.
 *
 *  @author Enrique Naudon (esn)
 **/

 #include <x86/seg.h>

.global mode_switch
mode_switch:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  # Load args
  movl  0x8(%ebp), %ecx           # Move user-mode EIP into ECX
  movl  0xC(%ebp), %edx           # Move user-mode ESP into EBX

  # Setup various data segment registers
  mov   $SEGSEL_USER_DS, %ax      # Can't move directly into segment regs
  mov   %ax, %ds                  # Set DS data segment
  mov   %ax, %es                  # Set ES data segment
  mov   %ax, %fs                  # Set FS data segment
  mov   %ax, %gs                  # Set GS data segment

  # Setup stack for iret
  pushl $SEGSEL_USER_DS           # Push user stack segment selector
  pushl %edx                      # Push user stack pointer
  pushf                           # Push flag register
  pushl $SEGSEL_USER_CS           # Push user code segment selector
  pushl %ecx                      # Push user program counter
  iret                             # To user-space!

  # We shouldn't return from IRET


