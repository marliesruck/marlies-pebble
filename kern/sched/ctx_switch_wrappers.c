#include <ctx_switch.h>

/* Pebbles specific includes */
#include <thread.h>
#include <sched.h>

/* Libc specific */
#include <assert.h>

/* x86 specific include */
#include <asm.h>

void dispatch_wrapper(thread_t *next)
{
  thread_t *prev;

  prev = curr;
  curr = next;

  ctx_switch(&prev->sp, &prev->pc, next->sp, next->pc,
               next->task_info->cr3, &next->kstack[KSTACK_SIZE]);
  return;
}
