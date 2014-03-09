/** @files asm_ctx_switch.S
 *
 *  @brief Assembly routine for context switches
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug We could pass the address of the prev and next tcb if we want to reduce
 *  the number of args we are passing to asm_ctx_switch
 **/
.extern set_esp0

.global asm_ctx_switch

asm_ctx_switch:
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

.global store_and_switch

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

