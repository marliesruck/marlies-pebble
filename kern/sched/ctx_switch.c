#include <ctx_switch.h>
#include <process.h>

#include <x86/eflags.h>
#define EFL_USER_MODE (EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF) & ~EFL_AC

#include <simics.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3);
/* All args are in registers to preserve the stack */
void store_and_switch(void);

int i = 0;

void ctx_switch(void)
{
  /* Set the flags for the first ctx switch because we haven't launched this
   * task yet */
  if(i == 0) set_eflags(EFL_USER_MODE);

  if(i%2 == 0){
    i++;
    asm_ctx_switch(&your_pcb.my_tcb.sp, &your_pcb.my_tcb.pc,
                 my_pcb.my_tcb.sp, my_pcb.my_tcb.pc, my_pcb.pg_dir);
  }
  else{
    lprintf("ctx switching back to first task");
    i++;
    asm_ctx_switch(&my_pcb.my_tcb.sp, &my_pcb.my_tcb.pc,
                 your_pcb.my_tcb.sp, your_pcb.my_tcb.pc, your_pcb.pg_dir);
  }
  /*
  asm_ctx_switch(&my_pcb[0].my_tcb.sp, &my_pcb[0].my_tcb.pc,
                 my_pcb[1].my_tcb.sp, my_pcb[1].my_tcb.pc, my_pcb[1].pg_dir);
                 */
  return;
}

/* If we load in a new task (i.e. via exec) and we want to 
void launch_new_task(void){
*/

