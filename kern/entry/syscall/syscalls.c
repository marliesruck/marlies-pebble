/** @file syscalls.c
 *
 *  @brief Implements our system calls.
 *
 *  Even though any two system calls might do two VERY different things, we
 *  decided to implement them all in one file so that our user-facing API
 *  is in one place.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <simics.h>

#include <assert.h>
#include <string.h>

#include <process.h>
#include <thread.h>
#include <ctx_switch.h>
#include <idt.h>
#include <syscall_int.h>
#include "syscall_wrappers.h"
#include "sc_utils.h"

/* For validating the file for exec */
#include <loader.h>

#include <string.h>

/** @brief Installs our system calls.
 *
 *  @return Void.
 **/
void install_sys_handlers(void)
{
  install_trap_gate(FORK_INT, asm_sys_fork, IDT_USER_DPL);
  install_trap_gate(EXEC_INT, asm_sys_exec, IDT_USER_DPL);
  install_trap_gate(SET_STATUS_INT, asm_sys_set_status, IDT_USER_DPL);
  install_trap_gate(VANISH_INT, asm_sys_vanish, IDT_USER_DPL);
  install_trap_gate(WAIT_INT , asm_sys_wait, IDT_USER_DPL);
  install_trap_gate(TASK_VANISH_INT, asm_sys_task_vanish, IDT_USER_DPL);
  install_trap_gate(GETTID_INT, asm_sys_gettid, IDT_USER_DPL);
  install_trap_gate(YIELD_INT, asm_sys_yield, IDT_USER_DPL);
  install_trap_gate(DESCHEDULE_INT, asm_sys_deschedule, IDT_USER_DPL);
  install_trap_gate(MAKE_RUNNABLE_INT, asm_sys_make_runnable, IDT_USER_DPL);
  install_trap_gate(GET_TICKS_INT, asm_sys_get_ticks, IDT_USER_DPL);
  install_trap_gate(SLEEP_INT , asm_sys_sleep, IDT_USER_DPL);
  install_trap_gate(NEW_PAGES_INT, asm_sys_new_pages, IDT_USER_DPL);
  install_trap_gate(REMOVE_PAGES_INT, asm_sys_remove_pages, IDT_USER_DPL);
  install_trap_gate(GETCHAR_INT, asm_sys_getchar, IDT_USER_DPL);
  install_trap_gate(READLINE_INT, asm_sys_readline, IDT_USER_DPL);
  install_trap_gate(PRINT_INT, asm_sys_print, IDT_USER_DPL);
  install_trap_gate(SET_TERM_COLOR_INT, asm_sys_set_term_color, IDT_USER_DPL);
  install_trap_gate(SET_CURSOR_POS_INT, asm_sys_set_cursor_pos, IDT_USER_DPL);
  install_trap_gate(GET_CURSOR_POS_INT, asm_sys_get_cursor_pos, IDT_USER_DPL);
  install_trap_gate(HALT_INT, asm_sys_halt, IDT_USER_DPL);
  install_trap_gate(READFILE_INT, asm_sys_readfile, IDT_USER_DPL);
  install_trap_gate(SWEXN_INT, asm_sys_swexn, IDT_USER_DPL);
  install_trap_gate(MISBEHAVE_INT, asm_sys_misbehave, IDT_USER_DPL);

  return;
}


/*************************************************************************
 *  Life cycle
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
#include <tlb.h>
#include <sched.h>
#include <frame_alloc.h>
#define CHILD_PDE ( (void *)tomes[PG_TBL_ENTRIES - 2] )
void *init_child_tcb(void *child_cr3, pte_s *pd, pt_t *pt)
{
  /* Initialize vm struct */
  thread_t *thread = task_init();
  task_t *task = thread->task_info;

  /* Initialize pg dir */
  task->cr3 = (uint32_t)(child_cr3);
  task->vmi.pg_info.pg_tbls = pt;
  task->vmi.pg_info.pg_dir = pd;
  //(task_t *)(task->parent) = (task_t *)(curr->task_info);

  return thread;
}
int finish_fork(void);
int sys_fork(unsigned int esp)
{
  pte_s pde;
  pt_t *child_pg_tables = (pt_t *)(CHILD_PDE);
  pte_s *child_pd = child_pg_tables[PG_SELFREF_INDEX];

  /* Map the child's PD into the parent's address space */
  void *child_cr3 = alloc_frame();
  init_pte(&pde, child_cr3);
  pde.present = 1;
  pde.writable = 1;
  task_t *curr_task = curr->task_info;
  set_pde(curr_task->vmi.pg_info.pg_dir, child_pd, &pde);
  set_pte(curr_task->vmi.pg_info.pg_dir, curr_task->vmi.pg_info.pg_tbls,
          child_pd, &pde);

  /* Initialize child stuffs */
  init_pd(child_pd, child_cr3);
  thread_t *child_thread = init_child_tcb(child_cr3, child_pd, child_pg_tables);
  task_t *child_task = child_thread->task_info;
  vm_copy(&child_task->vmi, &curr_task->vmi);

  /* Unmap child PD */
  init_pte(&pde, NULL);
  pde.present = 0;
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

  /* Increment the number of children forked */
  curr->task_info->live_children++;

  /* Enqueue new tcb */
  assert( thrlist_add(child_thread) == 0 );
  assert( sched_unblock(child_thread, 1) == 0 );

  return child_thread->tid;
}

#include <loader.h>
#include <usr_stack.h>
#include <vm.h>
#include <malloc.h>
void mode_switch(void *entry_point, void *sp);
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
  return;
}

#include <process.h>
#include <thread.h>
#include <sched.h>
#include <mutex.h>
#include <cvar.h>

#include <asm.h>

/* Free a thread's resources
 *
 * There are 2 cases:
 *
 * 1) There are remaining threads in the task.  Only free the calling thread's
 *    kernel stack and thread_t struct. This will be implemented in several
 *    iterations:
 *
 *    Iteration 1:
 *    -Enqueue in dead_peers so we can still use the kstack
 *    -call half ctx switch(not that our kstack is still valid because another
 *    process has not taken it)
 *
 *    Iteration 2:
 *    -interject scheduler logic here and acquire scheduler lock
 *    -call free_tcb(void *thread_t, void *next_task sp, void next_task *pc,
 *                  void *listp)
 *      -this will be an asm function  
 *        -*listp = thread_t
 *        -jmp asm_half_ctx_switch
 *       
 * 2) The calling thread is the last thread.  
 *    -Add any unreaped children to init's list of unreaped children
 *    -cond signal the parent, if this returns -1 then the parent already
 *    exited. Add your pcb to init's dead children linked list
 *
 *    Assumes: Parent is responsible for freeing the child's page directory and
 *    tables as well as the linked list of tcbs and the pcb.
 *
 */
void sys_vanish(void)
{
  /* Function is not ctx switch safe beyond this point */
  //mutex_lock(&sched_lock);
  disable_interrupts();

  /* Remove yourself from the runnable queue */
  remove_from_runnable(curr);

  /* There should still be someone to switch to */
  assert( !queue_empty(&runnable) );

  /* A little bit of scheduler logic so we can switch to the next thread */
  queue_node_s *q = queue_dequeue(&runnable);
  thread_t *next = queue_entry(thread_t *, q);

  /* Move the thread to the back of the queue */
  assert(next->state == THR_RUNNING);
  queue_enqueue(&runnable, q);

  curr = next; 

  half_ctx_switch(next->sp, next->pc, next->task_info->cr3,
                  &next->kstack[KSTACK_SIZE]);

  assert(0); /* Should never reach here */
  return;
}

int sys_wait(int *status_ptr)
{
  return -1;
}

void sys_task_vanish(int status)
{
  return;
}


/*************************************************************************
 *  Thread management
 *************************************************************************/

#include <sched.h>
#include <thread.h>


int sys_gettid(void)
{
  return curr->tid;
}

int sys_yield(int pid)
{
  return -1;
}

int sys_deschedule(int *reject)
{
  mutex_lock(&curr->lock);

  /*  TODO: check that reject is valid. */
  if (*reject) return 0;

  sched_mutex_unlock_and_block(curr, &curr->lock);
  schedule();

  return 0;
}

int sys_make_runnable(int tid)
{
  thread_t *thr;
  int ret;

  /* Find and lock the target */
  thr = thrlist_find(tid);
  mutex_lock(&thr->lock);

  if (!thr || thr->state == THR_RUNNING)
    return -1;

  ret = sched_unblock(thr, 1);
  mutex_unlock(&thr->lock);

  return ret;
}

#include <timer.h>
unsigned int sys_get_ticks(void)
{
  return tmr_get_ticks();
}

int sys_sleep(int ticks)
{
  return -1;
}


/*************************************************************************
 *  Memory management
 *************************************************************************/

#include <vm.h>


int sys_new_pages(void *addr, int len)
{
  if (!vm_alloc(&curr->task_info->vmi, addr, len,
                VM_ATTR_RDWR|VM_ATTR_USER|VM_ATTR_NEWPG))
  {
    return -1;
  }

  return 0;
}

int sys_remove_pages(void *addr)
{
  return -1;
}


/*************************************************************************
 *  Console I/O
 *************************************************************************/

#include <console.h>
#include <keyboard.h>


char sys_getchar(void)
{
  char ch;
  while ((ch = kbd_getchar()) == -1) continue;
  return ch;
}

int sys_readline(int size, char *buf)
{
  int count = 0;
  char ch = '\0';

  while (count < size && ch != '\n')
  {
    /* Read characters from the keyboard */
    while ((ch = kbd_getchar()) == -1) continue;

    /* Write to user buffer and console*/
    buf[count] = ch;
    putbyte(ch);

    /* Inc/Decrement read count */
    if (ch == '\b')
      --count;
    else
      ++count;
  }

  return count;
}

int sys_print(int size, char *buf)
{
  char *buf_k;

  /* Copy buf_k from user-space */
  buf_k = malloc(size);
  if (!buf_k) return -1;
  if (copy_from_user(buf_k, buf, size)) {
    free(buf_k);
    return -1;
  }

  /* Write to the console */
  putbytes(buf, size);

  free(buf_k);
  return 0;
}

int sys_set_term_color(int color)
{
  return set_term_color(color);
}

int sys_set_cursor_pos(int row, int col)
{
  return set_cursor(row, col);
}

int sys_get_cursor_pos(int *row, int *col)
{
  int krow, kcol;

  /* Get cursor coords */
  get_cursor(&krow, &kcol);

  /* Copy coords to user */
  *row = krow;
  *col = kcol;

  return 0;
}


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

void sys_halt()
{
  return;
}

int sys_readfile(char *filename, char *buf, int count, int offset)
{
  return -1;
}

#include <ureg.h>
typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  return -1;
}


/* "Special" */
void sys_misbehave(int mode)
{
  return;
}

