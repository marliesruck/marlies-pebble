/** @file atomic.S
 *
 *  @brief Implements various atomic operations for x86 in a C-callable
 *  fashion.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bug
 **/


/** @brief Implements fetch-and-add using the xadd.
 *
 *  @param addr The address of the memory we're fetching.
 *  @param addend The value to add to *addr (the augend).
 *
 *  @return The original value at addr.
 **/
.global fetch_and_add
fetch_and_add:
  # Prologue
  push  %ebp                      # Store old EBP
  mov   %esp, %ebp                # Set up new EBP

  movl  0x8(%ebp), %edx           # Move augend's address into EDX
  movl  0xC(%ebp), %eax           # Move addend into EAX
  lock xaddl %eax, (%edx)         # EAX = *EDX; *EDX = *EDX + EAX

  # Epilogue
  pop   %ebp                      # Restore old EBP
  ret                             # Return to caller


/** @brief Implements compare-and-swap using the cmpxchg.
 *
 *  @param addr The address of the memory to perfroming the CAS.
 *  @param expect The value we expect to see in *addr.
 *  @param new The value to (maybe) write into *addr.
 *
 *  @return The original at addr.
 **/
.global compare_and_swap
compare_and_swap:
  # Prologue
  push  %ebp                      # Store old EBP
  mov   %esp, %ebp                # Set up new EBP

  movl  0x8(%ebp), %edx           # Move pointer into EDX
  movl  0xC(%ebp), %eax           # Move expected value into EAX
  movl  0x10(%ebp), %ecx          # Move new value into EAX
  lock cmpxchg %ecx, (%edx)       # *EDX = (EAX == *EDX) ? ECX : *EDX

  # Epilogue
  pop   %ebp                      # Restore old EBP
  ret                             # Return to caller


