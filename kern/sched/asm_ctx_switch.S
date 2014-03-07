.extern set_esp0

.global asm_ctx_switch
asm_ctx_switch:
  # Prologue
  push %ebp
  mov %esp, %ebp

  pusha                 # Store gp
  
  # Set next thread's esp0
  mov 0x1c(%ebp), %ebx  # new_kstack_high
  push %ebx
  call set_esp0
  pop  %ebx

  # Liberally clobber all regs since we already stored them all out
  mov 8(%ebp), %eax     # &my_sp
  mov 0xc(%ebp), %ecx   # &my_pc
  mov 0x10(%ebp), %edx  # new_sp
  mov 0x14(%ebp), %esi  # new_pc
  mov 0x18(%ebp), %edi  # new_cr3

  call store_and_switch       

  popa                  # Restore gp

  # Epilogue
  pop %ebp
  ret

.global store_and_switch
store_and_switch:

  # Prologue
  push %ebp
  mov %esp, %ebp

  # Store out current thread's context
  leal 8(%ebp), %ebx   # Compute old ESP
  mov %ebx, (%eax)     # Store old ESP in &my_sp 
  mov 4(%ebp), %ebx    # Retrieve RA from stack
  mov %ebx, (%ecx)     # Store RA in &my_pc

  # Restore new thread's context
  mov %edx, %esp       # Restore ESP
  mov %edi, %cr3       # Load correct PTBR

  jmp *%esi            # Jump to new thread's EIP

