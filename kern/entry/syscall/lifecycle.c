/** @file lifecycle.c
 *
 *  @brief Implements our life-cycle system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include <sc_utils.h>

/* Pebble includes */
#include <cvar.h>
#include <dispatch.h>
#include <frame_alloc.h>
#include <loader.h>
#include <mutex.h>
#include <process.h>
#include <sched.h>
#include <thread.h>
#include <tlb.h>
#include <usr_stack.h>
#include <vm.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <string.h>

/* x86 includes */
#include <x86/asm.h>


/*************************************************************************
 *  Internal helper functions
 *************************************************************************/

/* For getting new kid threads back to user-space */
int asm_child_finish_sys_fork(void);
int asm_child_finish_sys_thread_fork(void);

void *kstack_copy(char *dst, char *src, unsigned int esp)
{
  /* Copy the parent's kstack */
  unsigned int offset = esp - ((unsigned int) src);
  size_t len = ((unsigned int) &src[KSTACK_SIZE]) - esp;
  memcpy(&dst[offset], (void *)esp, len);

  return &dst[offset];
}


/*************************************************************************
 *  Life cycle system calls
 *************************************************************************/

/* @brief Allocate memory for child's pcb
 *
 * This function is responsible for:
 * -Grab a tid (atomically)
 * -Store the tid/cr3 in the new pcb
 *
 * @bug For the sake of debugging I'm just using a global pcb2 declared in
 * process.h.  However, this is certainly a function we should study when
 * contemplating our locking strategy. We should probably add a state field to
 * our tcb in case the parent forks and them immediately waits on the child.  I
 * imagine the code might be reminiscent of thread_fork and setting the NASCENT
 * state
 *
 * @param child_cr3 Physical address of child's page directory
 * @return Address of malloc'd child pcb 
 */
int sys_fork(unsigned int esp)
{
  task_t *parent, *ctask;
  thread_t *cthread;
  void *sp, *pc;

  parent = curr->task_info;

  cthread = task_init();
  if(!cthread) return -1;
  ctask = cthread->task_info;

  /* Copy address space */
  assert(vm_copy(&ctask->vmi, &parent->vmi) == 0);

  /* Atomically increment live children in case you are vying with another
   * thread who is forking */
  mutex_lock(&parent->lock);
  parent->live_children++;
  mutex_unlock(&parent->lock);

  /* Launch the child thread */
  sp = kstack_copy(cthread->kstack, curr->kstack, esp);
  pc = asm_child_finish_sys_fork;
  thr_launch(cthread, sp, pc);

  return cthread->tid;
}

int sys_thread_fork(unsigned int esp)
{
  thread_t *t;
  void *sp, *pc;

  t = thread_init(curr->task_info);
  if (!t) return -1;

  if (task_add_thread(curr->task_info, t)) {
    free(t);
    return -1;
  }

  sp = kstack_copy(t->kstack, curr->kstack, esp);
  pc = asm_child_finish_sys_thread_fork;
  thr_launch(t, sp, pc);

  return t->tid;
}

int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  void *entry, *stack;
  simple_elf_t se;
  int i, j;

  /* Copy execname from user-space */
  execname_k = malloc(strlen(execname) + 1);
  if (!execname_k) return -1;

  /* Invalid memory or filename */
  if ((copy_from_user(execname_k, execname, strlen(execname) + 1))
      || (validate_file(&se, execname) < 0)){
      free(execname_k);
      return -1;
  }

  /* Copy argvec from user-space */
  for (i = 0; argvec[i] != NULL; ++i) continue;
  argvec_k = malloc((i + 1) * sizeof(char *));
  for (i = 0; argvec[i] != NULL; ++i)
  {
    argvec_k[i] = malloc(strlen(argvec[i]) + 1);
    if (copy_from_user(argvec_k[i], argvec[i], strlen(argvec[i]) + 1)) {
      free(execname_k);
      for (j = 0; j < i; ++j) free(argvec_k[j]);
      free(argvec_k);
      return -1;
    }
  }
  argvec_k[i] = NULL;

  /* Destroy the old address space; setup the new */
  vm_final(&curr->task_info->vmi);
  entry = load_file(&curr->task_info->vmi, execname_k);
  sim_reg_process(&curr->task_info->cr3, execname_k);
  stack = usr_stack_init(&curr->task_info->vmi, argvec_k);

  /* Free copied parameters*/
  for (j = 0; j < i; ++j) free(argvec_k[j]);
  free(argvec_k);
  free(execname_k);

  /* Execute the new program */
  half_dispatch(entry, stack);

  return 0;
}

void sys_set_status(int status)
{
  mutex_lock(&curr->task_info->lock);
  curr->task_info->status = status;
  mutex_unlock(&curr->task_info->lock);
  return;
}

void sys_vanish(void)
{
  assert(thrlist_del(curr) == 0);

  task_t *task = curr->task_info;

  /* Decrement the number of living threads */
  mutex_lock(&task->lock);
  int live_threads = (--task->num_threads);
  mutex_unlock(&task->lock);

  /* You are the last thread. Tell your parent to reap you */
  if(live_threads == 0){

    /* Free your resources */
    task_free(task);

    /* Wake your parent if he/she is waiting on you and
     * remove yourself from the runnable queue */
    task_signal_parent(task);
  }
  /* Remove yourself from the runnable queue */
  else{
    sched_block(curr);
  }

  assert(0); /* Should never reach here */
  return;
}

int sys_wait(int *status_ptr)
{
  task_t *child_task = task_find_zombie(curr->task_info);

  /* You have no children to reap */
  if(!child_task) return -1;

  /* Store out root task tid to return */
  int tid = child_task->tid;
 
  /* Scribble to status */
  if(status_ptr)
    *status_ptr = child_task->status;

  task_reap(child_task);

  return tid;
}

void sys_task_vanish(int status)
{
  return;
}

