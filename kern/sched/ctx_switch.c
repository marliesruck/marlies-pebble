#include <ctx_switch.h>
#include <process.h>
#include <thread.h>

#include <x86/eflags.h>
#define EFL_USER_MODE (EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF) & ~EFL_AC

#include <simics.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3, unsigned int kstack_high);
/* All args are in registers to preserve the stack */
void store_and_switch(void);

int i = 0;

void ctx_switch(void)
{
  /* Set the flags for the first ctx switch because we haven't launched this
   * task yet */
  if(i%2 == 0){
    i++;
    curr = &thread2;
    asm_ctx_switch(&thread1.sp, &thread1.pc, thread2.sp, 
                    thread2.pc, task2.cr3, (unsigned int)
                    (&thread2.kstack[KSTACK_SIZE -1]));
  }
  else{
    i++;
    curr = &thread1;
    asm_ctx_switch(&thread2.sp, &thread2.pc, thread1.sp, 
                    thread1.pc, task1.cr3, (unsigned int)
                    (&thread1.kstack[KSTACK_SIZE -1]));
  }
  return;
}
