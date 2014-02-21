#include <x86/seg.h>
#include <x86/cr.h>

.global mode
mode:
  push %ebp
  mov %esp, %ebp

/*  push %esp */
/*  call set_esp0 */

  movl $SEGSEL_USER_DS, %eax    /* Load up the data segment selectors */
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs

  push $SEGSEL_USER_DS            /* Push SS */
  push 8(%ebp)                    /* Push user %esp */  
  push 0xc(%ebp)                  /* Push EFLAGS */
  movl $SEGSEL_USER_CS, %eax      /* Push CS */
  pushl $0x1000000                /* Push user entry point */
  iret


