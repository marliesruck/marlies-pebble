.global finish_fork

finish_fork:

  # Restore data segments
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment

  # Restore callee-saved registers
  pop %ebx
  pop %edi
  pop %esi

  # Return 0
  xor %eax,%eax

  # To userland!
  iret
