/** @file asm_thr.S
 *
 *  @brief Defines various assembly functions for our thread library.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 */

#include <syscall_int.h>

.extern child_init


/** @brief Fork a child thread.
 *
 *  After the fork, the child sets up it's stack before calling the child
 *  entry function, child_init(...); the parent simply returns.  To be
 *  clear, the child DOES NOT return from this function.  Also note that
 *  this function assumes that the arguments to child_init(...) have
 *  already been pushed onto the child's stack.
 *
 *  @param stack The TOP of the child's stack.
 *
 *  @return The child's TID to the parent; the child does not return.
 */
.global thread_fork
thread_fork:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  # Perform thread forking
  mov   8(%ebp),%edx              # Move child's ESP in a callee save reg
  int   $THREAD_FORK_INT          # Fork (CHILD CAN'T USE STACK AFTER THIS)
  test 	%eax, %eax                # Parent get's child TID; child gets 0
  jne   .parent                   # Split parent and child execution

  # Child does stack setup
  .child:
    mov   %edx, %esp              # Point ESP at the child's stack
    mov   %esp, %ebp              # Point EBP at the same place
    call  child_init             # Set up the child

  # Parent just returns
  .parent:
    pop   %ebp                    # Restore old EBP
    ret                           # Return


/** @brief Remove the specified memory and destroy the caller.
 *
 *  Assuming the call to remove_pages(...) succeeds, the removed address is
 *  writen to the pointer specified by the listp parameter.  This gives us
 *  a way to reuse stack addresses safely: they will not be written to
 *  listp until remove pages is called, and ONLY if it succeeds.
 *
 *  @param base The base of the stack to remove.
 *  @param listp A pointer to write the removed stack base.
 *
 *  @return Does not return.
 */
.global remove_and_vanish
remove_and_vanish:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP
  push  %esi                      # Save ESI (we <3 calling conventions!)

  # Load args
  movl  0x8(%ebp), %esi           # Move stack base into ESI
  movl  0xC(%ebp), %ebx           # Move list ptr into EBX

  # Recycle the stack
  int   $REMOVE_PAGES_INT         # Remove memory page(s)
  test  %eax, %eax                # We hope remove_pages succeeds
  jnz   .vanish                   # Don't reuse the stack if remove fails
  mov   %esi, (%edx)              # Store stack address in the free list

  # Vanish
  .vanish:
    int   $VANISH_INT             # Destroy the current task

  # Vanish won't return


/** @brief Return the value in ESP.
 *
 *  @return ESP.
 */
.global get_esp
get_esp:

  # Prologue
  push  %ebp                      # Store old EBP
  movl  %esp, %ebp                # Set up new EBP

  mov   %esp,%eax                 # Return ESP

  # Epilogue
  pop   %ebp                      # Restore old EBP
  ret                             # Return

