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
#include <idt.h>
#include <syscall_int.h>
#include "syscall_wrappers.h"
#include "sc_utils.h"


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
#define CHILD_PDE (void *)(0xF0000000)
void *init_child_tcb(void *child_cr3)
{
  /* Initialize vm struct */
  pcb2.vmi = (vm_info_s) {
    .pg_info = (pg_info_s) {
      .pg_dir = ((pt_t *)(CHILD_PDE))[PG_TBL_ENTRIES - 1],
      .pg_tbls = (pt_t *)(CHILD_PDE),
    },
    .mmap = CLL_LIST_INITIALIZER(pcb2.vmi.mmap)
  };

  /* Initialize pg dir and tid in prototype tcb */
  pcb2.cr3 = (uint32_t) (child_cr3);
  pcb2.my_tcb.tid = 5;

  /* Set state to RUNNABLE */

  return NULL;
}

int finish_fork(void);
int sys_fork(unsigned int esp)
{
#include <frame_alloc.h>
  pte_s pde;
  pt_t *child_pg_tables = (pt_t *)(CHILD_PDE);
  pte_s *child_pd = child_pg_tables[PG_TBL_ENTRIES - 1];
  
  /* Allocate a frame for the child PD */
  void *child_cr3 = alloc_frame();

  /* Map it into the parent's address space */
  init_pte(&pde, child_cr3);
  pde.present = 1;
  pde.writable = 1;
  set_pde(curr_pcb->vmi.pg_info.pg_dir, child_pd, &pde);
  set_pte(curr_pcb->vmi.pg_info.pg_dir, curr_pcb->vmi.pg_info.pg_tbls,
          child_pd, &pde);

  /* Initialize child stuffs */
  init_pd(child_pd, child_cr3);
  init_child_tcb(child_cr3);
  vm_copy(&pcb2.vmi, &curr_pcb->vmi);

  /* Copy the parent's kstack */
  unsigned int offset = esp - ((unsigned int) curr_pcb->my_tcb.kstack);
  size_t len = ((unsigned int) &curr_pcb->my_tcb.kstack[KSTACK_SIZE]) - esp;
  void *dest = &pcb2.my_tcb.kstack[offset];
  memcpy(dest, (void *)esp, len);

  /* Last touches to the child PCB */
  pcb2.vmi.pg_info.pg_dir = (pte_s *)(TBL_HIGH);
  pcb2.vmi.pg_info.pg_tbls = (pt_t *)(DIR_HIGH);
  pcb2.my_tcb.sp = &pcb2.my_tcb.kstack[offset];
  pcb2.my_tcb.pc = finish_fork;

  /* Atomically insert child into runnable queue */

  return pcb2.my_tcb.tid;
}

#include <loader.h>
#include <usr_stack.h>
#include <vm.h>
#include <malloc.h>
#include <string.h>
void mode_switch(void *entry_point, void *sp);
int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  int i, j;

  /* Copy execname from user-space */
  execname_k = malloc(strlen(execname) + 1);
  if (!execname_k) return -1;
  if (copy_from_user(execname_k, execname, strlen(execname) + 1)) {
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
  vm_final(&curr_pcb->vmi);
  void *entry_point = load_file(&curr_pcb->vmi, execname_k);
  void *usr_sp = usr_stack_init(&curr_pcb->vmi, argvec_k);

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

void sys_vanish(void)
{
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

int sys_gettid(void)
{
  return curr_pcb->my_tcb.tid;
}

int sys_yield(int pid)
{
  return -1;
}

int sys_deschedule(int *flag)
{
  return -1;
}

int sys_make_runnable(int pid)
{
  return -1;
}

unsigned int sys_get_ticks(void)
{
  return 0;
}

int sys_sleep(int ticks)
{
  return -1;
}


/*************************************************************************
 *  Memory management
 *************************************************************************/

int sys_new_pages(void *addr, int len)
{
  return -1;
}

int sys_remove_pages(void *addr)
{
  return -1;
}


/*************************************************************************
 *  Console I/O
 *************************************************************************/

/* REMOVED FOR DEBUGGING, REVERT THIS
char sys_getchar(void)
{
  return -1;
}
*/

int sys_readline(int size, char *buf)
{
  return -1;
}

int sys_print(int size, char *buf)
{
  return -1;
}

int sys_set_term_color(int color)
{
  return -1;
}

int sys_set_cursor_pos(int row, int col)
{
  return -1;
}

int sys_get_cursor_pos(int *row, int *col)
{
  return -1;
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

