/** @file tlb.S
 *
 *  @brief Implements TLB-related assembly routines.
 *
 *  @author Enrique Naudon (esn)
 **/


.global tlb_inval_page
tlb_inval_page:

  # Prologue
  pushl %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  # Flush the TLB
  movl 0x8(%ebp), %eax            # Load the address
  invlpg (%eax)                   # Flush it from the TLB

  # Epilogue
  popl  %ebp                      # Restore old EBP
  ret                             # Return to caller


