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
#include <ctx_switch.h>
#include <cvar.h>
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

#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )

/*************************************************************************
 *  Internal helper functions
 *************************************************************************/

/* TODO: Maybe these get headers? */
int finish_fork(void);

void *init_child_tcb(void *child_cr3, pte_t *pd, pt_t *pt)
{
  /* Initialize vm struct */
  thread_t *thread = task_init();
  if(!thread)
    return NULL;
  task_t *task = thread->task_info;

  /* Initialize pg dir */
  task->cr3 = (uint32_t)(child_cr3);
  task->vmi.pg_info.pg_tbls = pt;
  task->vmi.pg_info.pg_dir = pd;
  task->parent = curr->task_info;

  return thread;
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
 * @return Address of malloced child pcb 
 */
int sys_fork(unsigned int esp)
{
  pte_t pde;
  pt_t *child_pg_tables = CHILD_PDE;
  pte_t *child_pd = child_pg_tables[PG_SELFREF_INDEX];
  task_t *curr_task = curr->task_info;

  /* Map the child's PD into the parent's address space */
  void *child_cr3 = alloc_page_table(&curr_task->vmi.pg_info, CHILD_PDE);
  pde = PACK_PTE(child_cr3, PG_SELFREF_ATTRS);
  set_pte(curr_task->vmi.pg_info.pg_dir, curr_task->vmi.pg_info.pg_tbls,
          child_pd, &pde);

  /* Initialize child stuffs */
  init_pd(child_pd, child_cr3);

  thread_t *child_thread = init_child_tcb(child_cr3, child_pd, child_pg_tables);

  task_t *child_task = child_thread->task_info;
  vm_copy(&child_task->vmi, &curr_task->vmi);

  /* Unmap child PD */
  pde = PACK_PTE(NULL, 0);
  set_pde(curr_task->vmi.pg_info.pg_dir, child_pd, &pde);
  tlb_inval_tome(child_pg_tables);
  tlb_inval_page(curr_task->vmi.pg_info.pg_tbls[PG_DIR_INDEX(child_pd)]);

  /* Copy the parent's kstack */
  unsigned int offset = esp - ((unsigned int) curr->kstack);
  size_t len = ((unsigned int) &curr->kstack[KSTACK_SIZE]) - esp;
  void *dest = &child_thread->kstack[offset];
  memcpy(dest, (void *)esp, len);

  /* Last touches to the child */
  child_task->vmi.pg_info.pg_dir = PG_TBL_ADDR[PG_SELFREF_INDEX];
  child_task->vmi.pg_info.pg_tbls = PG_TBL_ADDR;
  child_thread->sp = &child_thread->kstack[offset];
  child_thread->pc = finish_fork;

  /* Keep track of live children */
  curr_task->live_children++;

  /* Enqueue new tcb */
  assert( thrlist_add(child_thread) == 0 );
  assert( sched_unblock(child_thread, 1) == 0 );

  return child_thread->tid;
}
/* @bug Add a pcb final function for reinitializing pcb */
int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  int i, j;
  simple_elf_t se;

  /* Copy execname from user-space */
  execname_k = malloc(strlen(execname) + 1);
  if (!execname_k) return -1;

  /* Invalid memory or filename */
  if ((copy_from_user(execname_k, execname, strlen(execname) + 1)) || 
      (validate_file(&se, execname) < 0)){
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

  void *entry_point = load_file(&curr->task_info->vmi, execname_k);

  void *usr_sp = usr_stack_init(&curr->task_info->vmi, argvec_k);

  /* Free copied parameters*/
  for (j = 0; j < i; ++j) free(argvec_k[j]);
  free(argvec_k);
  free(execname_k);

  /* Execute the new program */
  mode_switch(entry_point, usr_sp);

  return 0;
}

void sys_set_status(int status)
{
  curr->task_info->status = status;
  return;
}

void sys_vanish(void)
{
  cll_node *n;

  thrlist_del(curr);

  task_t *task = curr->task_info;

  /* Decrement the number of living threads */
  mutex_lock(&task->lock);
  int live_threads = (--task->num_threads);
  mutex_unlock(&task->lock);

  /* You are the last thread. Tell your parent to reap you */
  if(live_threads == 0){

    /* Remove yourself from the task list so that your children cannot add
     * themselves to your dead children list */
    tasklist_del(task);

    /* Acquire and release your lock in case your child grabbed your task lock
     * before you removed yourself from the task list */
    mutex_lock(&task->lock);
    mutex_unlock(&task->lock);

    /* Free your virtual memory */
    vm_final(&task->vmi);

    /* Make your dead children the responsibility of init */
    while(!cll_empty(&task->dead_children)){
      n = cll_extract(&task->dead_children, task->dead_children.next);
      mutex_lock(&init->lock);
      cll_insert(&init->dead_children, n);
      mutex_unlock(&init->lock);
    }

    /* Check if your parent is still alive...
     * This function does NOT release the list lock so that you can add 
     * yourself as dead child before the parent exits (since you must
     * lock the task list to exit) */
    task_t *parent = task->parent;
    if(!tasklist_find(parent)) 
      parent = init; /* Your parent is dead. Tell init instead */
    else
    /* Live children count is for direct descendents only.  Don't do this if
     * your parent is init */
      parent->live_children--;

    /* Grab your parent's lock to prevent them from exiting while you are trying
     * to add yourself as a dead child */
    mutex_lock(&parent->lock);

    /* Release the list lock */
    mutex_unlock(&task_list_lock);

    /* Add yourself as a dead child */
    n = malloc(sizeof(cll_node));
    cll_init_node(n, task);
    cll_insert(&parent->dead_children, n);
    cvar_signal(&parent->cv);

    /* Your parent should not reap you until you've descheduled yourself */
    sched_mutex_unlock_and_block(curr, &parent->lock);
    schedule();
  }
  /* Remove yourself from the runnable queue */
  else{
    sched_block(curr);
    schedule();
  }

  assert(0); /* Should never reach here */
  return;
}

int sys_wait(int *status_ptr)
{
  cll_node *n;
  task_t *task = curr->task_info;

  /* Atomically check for dead children */
  mutex_lock(&task->lock);

  while(cll_empty(&task->dead_children)){
    if(task->live_children == 0){
        /* Avoid unbounded blocking */
        mutex_unlock(&task->lock);
        return 0;
      }
    else{
      cvar_wait(&task->cv, &task->lock);
    }
  }
 /* Remove dead child */
 n = cll_extract(&task->dead_children, task->dead_children.next);

 task_t *child_task = cll_entry(task_t*, n);
 free(n);

 /* Relinquish lock */
 mutex_unlock(&task->lock);

 /* Store out root task tid to return */
 int tid = child_task->orig_tid;

 /* Scribble to status */
 if(status_ptr)
   *status_ptr = child_task->status;

 /* Free child's page directory */
 free_unmapped_frame((void *)(child_task->cr3), &task->vmi.pg_info);

 /* Free child's thread resources... */
 cll_free(&child_task->peer_threads);

 /* Free child's task struct */
 free(child_task);

 return tid;
}

void sys_task_vanish(int status)
{
  return;
}

