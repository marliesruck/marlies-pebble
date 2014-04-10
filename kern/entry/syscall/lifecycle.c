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

/** @brief Copy the parent's kernel stack.
 *
 *  We only copy the top parent's kernel stack (just the stuff that the
 *  INT pushed), because any data below that (return addresses, stored
 *  EBPs, etc.) will not make sense on the child's kernel stack.  The
 *  child's return-to-user-space functions (asm_child_finish_*(...)) use
 *  the copied INT stuff to return to user-space.
 *
 *  @param dst A pointer to the child's kernel stack.
 *  @param src A pointer to the parent's kernel stack.
 *  @param esp The value of the parent's ESP.
 *
 *  @return The child's ESP.
 **/
void *kstack_copy(char *dst, char *src, unsigned int esp)
{
  unsigned int offset;
  size_t len;

  /* Calculate the offset of ESP from the top of there parent's stack */
  offset = esp - ((unsigned int) src);

  /* Calculate how much we need to copy */
  len = ((unsigned int) &src[KSTACK_SIZE]) - esp;

  /* Do the actual copying */
  memcpy(&dst[offset], (void *)esp, len);

  return &dst[offset];
}


/*************************************************************************
 *  Life cycle system calls
 *************************************************************************/

/** @brief Fork a child task.
 *
 *  Creates a new task with a copy the virtual memory of the parent for the
 *  child task.  The parent will recieve the TID of the root thread in the
 *  newly created task, and the child will recieve 0.  If the invoking task
 *  has more than one thread, sys_fork(...) will fail.
 *
 *  @param esp The value of ESP after the INT.
 *
 *  @return On success, the child's TID to the parent and 0 to the child.
 *  On failure, no new task is created and the parent recieves a negative
 *  integer error code.
 **/
int sys_fork(unsigned int esp)
{
  task_t *parent, *ctask;
  thread_t *cthread;
  void *sp, *pc;
  int tid, thr_count;

  parent = curr_tsk;

  /* Only single threaded tasks can fork */
  mutex_lock(&curr_tsk->lock);
  thr_count = parent->num_threads;
  mutex_unlock(&curr_tsk->lock);
  if(thr_count != 1)
    return -2;

  /* Create the new task */
  cthread = task_init();
  if(!cthread) return -1;
  tid = cthread->tid; 
  ctask = cthread->task_info;

  /* Copy address space */
  if (vm_copy(&ctask->vmi, &parent->vmi)) {
    free(ctask->mini_pcb);
    task_free(ctask);
    task_final(ctask);
    return -1;
  }

  /* Register the process with simics for debugging
   * TODO: is this a memory leak??
   */
  ctask->execname = parent->execname;
  sim_reg_process((void *)ctask->cr3, ctask->execname);

  /* Add the child to it's parent task */
  task_add_child(parent);

  /* Launch the child thread */
  sp = kstack_copy(cthread->kstack, curr_thr->kstack, esp);
  pc = asm_child_finish_sys_fork;
  thr_launch(cthread, sp, pc);

  return tid;
}

/** @brief Fork a new thread in the current task.
 *
 *  The new thread shares the virtual address space of the invoking thread,
 *  but has it's own set of registers, the values of which are initialized
 *  to the same value as the parent's registers.  Any software exception
 *  handler registered in the parent is NOT registered in the child.
 *
 *  Calls to sys_fork(...) or sys_exec(...) will fail in multi-threaded
 *  tasks.
 *
 *  @param esp The value of ESP after the INT.
 *
 *  @return On success, the new thread's TID to the parent and 0 to the
 *  child.  On failure, no new thread is created and the parent recieves a
 *  negative integer error code.
 **/
int sys_thread_fork(unsigned int esp)
{
  thread_t *t;
  void *sp, *pc;
  int tid;

  t = thread_init(curr_tsk);
  if (!t) return -1;
  tid = t->tid;

  /* Add a task to the thread */
  task_add_thread(curr_tsk);

  sp = kstack_copy(t->kstack, curr_thr->kstack, esp);
  pc = asm_child_finish_sys_thread_fork;
  thr_launch(t, sp, pc);

  return tid;
}

/** @brief Execute a new executable.
 *
 *  TODO: comment
 *
 *  @param execname The name of the new executable.
 *  @param argvec A NULL-terminated array of NULL-terminated string
 *  arguments.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  void *entry, *stack;
  simple_elf_t se;
  int argcnt, i;
  int thr_count;

  /* Only single threaded tasks can exec */
  mutex_lock(&curr_tsk->lock);
  thr_count = curr_tsk->num_threads;
  mutex_unlock(&curr_tsk->lock);
  if(thr_count != 1)
    return -2;

  /* Copy the execname from the user */
  if (copy_str_from_user(&execname_k, execname) < 0)
    return -1;

  /* Make sure it exists */
  if (validate_file(&se, execname_k) < 0) {
    free(execname_k);
    return -1;
  }

  /* Copy the argument vector from user */
  argcnt = copy_argv_from_user(&argvec_k, argvec);
  if (argcnt < 0) {
    free(execname_k);
    return -1;
  }

  /* Destroy the old address space; setup the new */
  vm_final(&curr_tsk->vmi);
  entry = load_file(&curr_tsk->vmi, execname_k);
  stack = usr_stack_init(&curr_tsk->vmi, argcnt, argvec_k);
  if (!entry || !stack) {
    for (i = 0; i < argcnt; ++i) free(argvec_k[i]);
    free(argvec_k);
    free(execname_k);
    return -1;
  }

  sim_reg_process(&curr_tsk->cr3, execname_k);

  /* Zero the status */
  TASK_STATUS(curr_tsk) = 0;

  /* Free copied parameters*/
  for (i = 0; i < argcnt; ++i) free(argvec_k[i]);
  free(argvec_k);

  /* Keep the new execname for debugging */
  curr_tsk->execname = execname_k;
  /* MEMORY LEAK */

  /* Execute the new program (should not return) */
  half_dispatch(entry, stack);
  assert(0);
  return 0;
}

/** @brief Set exit status of the current task.
 *
 *  @param status The new status.
 *
 *  @return Void.
 **/
void sys_set_status(int status)
{
  mutex_lock(&curr_tsk->lock);
  TASK_STATUS(curr_tsk) = status;
  mutex_unlock(&curr_tsk->lock);
  return;
}

/** @brief Kill the currently running thread.
 *
 *  The invoking thread deletes itself from it's task and, if it is the
 *  last thread in that task, deallocates as many of it's task's resources
 *  as possible and deschedules itself (permanently).
 *
 *  @return Void.
 **/
void sys_vanish(void)
{
  task_t *task = curr_tsk;
  mutex_s *lock = &task->lock;

  /* Remove yourself from the thread list */
  assert( thrlist_del(curr_thr) == 0 );

  mutex_lock(lock);

  /* Delete your thread from the task and reap a peer thread */
  task_del_thread(task, curr_thr);

  /* You are the last thread */ 
  if (0 == task->num_threads)
  {
    /* You're the last thread, so there's no competition */
    mutex_unlock(lock);

    /* Destroy the task, enqueue your status and signal your parent */
    task_free(task);
    task_t *parent = tasklist_find_and_lock_parent(task);
    task_del_child(parent, task);

    /* Drop your parent's lock upon descheduling */
    lock = &parent->lock;
  }

  /* Drop whatever lock you hold and deschedule */
  sched_do_and_block(curr_thr, (sched_do_fn) mutex_unlock_raw, lock);
  return;
}

/** @brief Attempt to reap the exit status of a dead child task.
 *
 *  If there are no live children and no not-yet-reaped dead children, the
 *  invoking thread will return with an error code.  If there are live
 *  children, but no dead and not-yet-reaped children, the invoking thread
 *  will block until one of it's children dies.  If there are dead children
 *  waiting to be reaped, and no other thread is already waiting to reap a
 *  child, the invoking thread will return the TID if the child it reaped.
 *  The child's exit status will be writen to status_ptr.
 *
 *  @param status_ptr A pointer to write the child's exit status to.
 *
 *  @return The child's TID on success, or a negative integer error code if
 *  there are no children to be reaped.
 **/
int sys_wait(int *status_ptr)
{
  int tid, status;
  
  /* Attempt to reap a child */
  tid = task_reap(curr_tsk, &status);

  /* You have no children to reap */
  if(tid < 0) return -1;
 
  /* Scribble to status */
  if(status_ptr) {
    if (copy_to_user((char *)status_ptr, (char *)&status, sizeof(int)))
      return -1;
  }

  return tid;
}

/** @brief Destroy the entire task of the invoking thread.
 *
 *  NOTE: unimplemented
 *
 *  @param status The task's exit status.
 **/
void sys_task_vanish(int status)
{
  return;
}

