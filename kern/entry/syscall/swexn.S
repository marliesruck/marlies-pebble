/** @file swexn.S
 *
 *  @brief Craft register contents before returning from swexn.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <x86/eflags.h>
#define EFL_USER_MODE ((EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF) & ~EFL_AC)

.global craft_state

craft_state:

  # Throw away return address, cause, cr2
  add   $12, %esp                 

  # Populate data registers
  pop   %ds
  pop   %es
  pop   %fs
  pop   %gs

  # Populate GP registers
  popa

  # Throw away error code
  add    $4, %esp

  # To user land! 
  iret

.global run_handler

run_handler:

  # Prologue
  push  %ebp
  mov   %esp, %ebp

  push EFL_USER_MODE
  call set_eflags

  # Retrieve arguments
  mov   0x8(%ebp), %eax           # Retrieve address of the handler
  mov   0xc(%ebp), %esp           # Set up user defined ESP

  jmp   *%eax                     # Jump to the handler 

.global get_ebp

get_ebp:
  mov   %ebp, %eax
  ret









