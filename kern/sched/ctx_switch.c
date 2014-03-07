#include <ctx_switch.h>
#include <process.h>
#include <thread.h>

#include <simics.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3, void *kstack_high);
/* All args are in registers to preserve the stack */
void store_and_switch(void);

int i = 0;

void ctx_switch(void)
{
  thread_t *prev = curr;
  thrlist_enqueue(prev, &naive_thrlist);
  curr = thrlist_dequeue(&naive_thrlist);

  if(i%2 == 0){
    i++;
    curr = thread2;
    asm_ctx_switch(&prev->sp, &prev->pc, curr->sp, 
                    curr->pc, curr->task_info->cr3, 
                    (&curr->kstack[KSTACK_SIZE -1]));

  }
  else{
    i++;
    curr = thread1;
    asm_ctx_switch(&prev->sp, &prev->pc, curr->sp, 
                    curr->pc, curr->task_info->cr3, 
                    (&curr->kstack[KSTACK_SIZE -1]));
  }
  return;
}
