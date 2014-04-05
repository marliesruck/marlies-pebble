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

/** @brief Program table */
struct prog {
  char *name;
  int expected;
  int pid;
  int count;
} progs[]
  = {
  {"exec_basic", 1, -1, 5},
  {"fork_wait", 0, -1, 5},
  {"loader_test1", 0, -1, 5},
  {"loader_test2", 42, -1, 5},
  {"make_crash",10000,-1, 1},
  {"print_basic",0,-1,5},
  {"remove_pages_test1", 0, -1, 2},
  {"sleep_test1", 42, -1, 5},  /* indistinguishible outcomes */
  {"stack_test1", 42, -1, 5},
  {"yield_desc_mkrun", 0, -1, 5},
  {"remove_pages_test2", -2, -1, 5}, /* Should die: wild pointer */
  {"wild_test1", -2, -1, 5},   /* Should die: wild pointer */
};

#include <string.h>

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
  int tid;

  parent = curr_tsk;

  cthread = task_init();
  if(!cthread) return -1;
  tid = cthread->tid; 
  ctask = cthread->task_info;

  /* Copy address space */
  assert(vm_copy(&ctask->vmi, &parent->vmi) == 0);

  /* Register the process with simics for debugging */
  ctask->execname = parent->execname;
  sim_reg_process((void *)ctask->cr3, ctask->execname);

  /* Zero the status */
  ctask->status = 0;

  /* Atomically increment live children in case you are vying with another
   * thread who is forking */
  mutex_lock(&parent->lock);
  parent->live_children++;
  mutex_unlock(&parent->lock);

  /* Launch the child thread */
  sp = kstack_copy(cthread->kstack, curr_thr->kstack, esp);
  pc = asm_child_finish_sys_fork;
  thr_launch(cthread, sp, pc);

  return tid;
}

int sys_thread_fork(unsigned int esp)
{
  thread_t *t;
  void *sp, *pc;
  int tid;

  t = thread_init(curr_tsk);
  if (!t) return -1;
  tid = t->tid;

  if (task_add_thread(curr_tsk, t)) {
    free(t);
    return -1;
  }

  sp = kstack_copy(t->kstack, curr_thr->kstack, esp);
  pc = asm_child_finish_sys_thread_fork;
  thr_launch(t, sp, pc);

  return tid;
}

int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  void *entry, *stack;
  simple_elf_t se;
  int i, j;

  /* Copy the execname from the user */
  if (copy_str_from_user(&execname_k, execname))
    return -1;

  /* Make sure it exists */
  if (validate_file(&se, execname_k) < 0) {
    free(execname_k);
    return -1;
  }

  /* Allocate kernel mem for argvec */
  mutex_lock(&curr_tsk->lock);
  if (!vm_find(&curr_tsk->vmi, (void *)argvec)) {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }
  for (i = 0; argvec[i] != NULL; ++i) continue;
  mutex_unlock(&curr_tsk->lock);
  argvec_k = malloc((i + 1) * sizeof(char *));
  if (!argvec_k) {
    free(execname_k);
    return -1;
  }

  /* Copy each arg from user-space */
  for (i = 0; argvec[i] != NULL; ++i)
  {
    if (copy_str_from_user(&argvec_k[i], argvec[i]))
    {
      for (j = 0; j < i; ++j) free(argvec_k[j]);
      free(argvec_k);
      free(execname_k);
      return -1;
    }
  }
  argvec_k[i] = NULL;

  /* Destroy the old address space; setup the new */
  vm_final(&curr_tsk->vmi);
  entry = load_file(&curr_tsk->vmi, execname_k);
  sim_reg_process(&curr_tsk->cr3, execname_k);
  stack = usr_stack_init(&curr_tsk->vmi, argvec_k);

  /* Zero the status */
  curr_tsk->status = 0;

  /* Free copied parameters*/
  for (j = 0; j < i; ++j) free(argvec_k[j]);
  free(argvec_k);

  /* Keep the new execname for debugging */
  curr_tsk->execname = execname_k;

  /* Execute the new program */
  half_dispatch(entry, stack);

  return 0;
}

void sys_set_status(int status)
{
  int i;
  int num_tests = 10;

  for(i = 0; i < num_tests; i++){
    if(!strcmp(curr_tsk->execname,progs[i].name)){
      if(status == -2){
        lprintf("set_status(): %s setting status -2!", progs[i].name);
        MAGIC_BREAK;
      }
    }
  }

  mutex_lock(&curr_tsk->lock);
  curr_tsk->status = status;
  mutex_unlock(&curr_tsk->lock);
  return;
}

void sys_vanish(void)
{
  task_t *task = curr_tsk;

  assert( thrlist_del(curr_thr) == 0 );

  mutex_lock(&task->lock);

  /* There are still live threads */
  if(0 < --task->num_threads){
    sched_mutex_unlock_and_block(curr_thr, &task->lock);
  }
  mutex_unlock(&task->lock);

  /* You were killed by the kernel */
  if(curr_thr->killed){
    int i;
    int num_tests = 10;

    for(i = 0; i < num_tests; i++){
      if(!strcmp(curr_tsk->execname,progs[i].name)){
          lprintf("vanish(): %s status -2!", progs[i].name);
          MAGIC_BREAK;
      }
    }
      task->status = -2;
  }

  /* You are the last thread */ 
  task_free(task);

  /*Tell your parent to reap you */
  task_signal_parent(task);

  return;
}

int sys_wait(int *status_ptr)
{
  task_t *child_task = task_find_zombie(curr_tsk);

  /* You have no children to reap */
  if(!child_task) return -1;

  /* Store out root task tid to return */
  int tid = child_task->tid;
 
  /* Scribble to status
   * TODO: what if this fails?
   */
  if(status_ptr) {
    if ( copy_to_user((char *)status_ptr, (char *)&child_task->status,
                     sizeof(int)) )
    {
      return -1;
    }
    if(*status_ptr == -2){
      int i;
      int num_tests = 10;
      for(i = 0; i < num_tests; i++){
        if(!strcmp(child_task->execname,progs[i].name)){
            lprintf("sys wait(): %s died status -2!", progs[i].name);
            MAGIC_BREAK;
        }
      }
    }
  }

  task_reap(child_task);

  return tid;
}

void sys_task_vanish(int status)
{
  return;
}

