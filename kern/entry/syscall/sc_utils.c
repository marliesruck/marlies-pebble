/** @file sc_utils.c
 *
 *  @brief Implements various system call-related utility functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include <sc_utils.h>
#include "syscall_wrappers.h"

/* Pebble includes */
#include <dispatch.h>
#include <idt.h>
#include <sched.h>
#include <syscall_int.h>
#include <ureg.h>
#include <util.h>
#include <vm.h>

/* Libc includes */
#include <assert.h>
#include <string.h>

/* x86 specific includes */
#include <x86/eflags.h>
#include <x86/seg.h>

/* Upper 10 bits of EFLAGS are reserved */
#define RESV 0xFFC00000

/* These bits in EFLAGS should be 1 */
#define EFL_SET (EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF)

/* These bits in EFLAGS should be 0 */
#define EFL_UNSET (EFL_RESV2 | EFL_RESV3 | EFL_IOPL_RING1  | EFL_IOPL_RING2 \
                  | EFL_IOPL_RING3 | EFL_RESV3 | EFL_AC | RESV)

/** @brief Implement the kernel thread killing policy.
 *
 *  @return Void.
 **/
void slaughter(void)
{
  /* You were killed by the kernel */
  curr_thr->killed = 1;

  /* Free your kernel resources */
  sys_vanish();

  return;
}

/** @brief Set up exception stack and deregister handler.
 *
 *  Since we want the handler to run in user mode, we copy the contents of the
 *  executation state onto the exception stack so that way it can modify it as
 *  it pleases. 
 *
 *  @param state Execution state to push on exception stack.
 *
 *  @return Void.
 **/
void init_exn_stack(ureg_t *state)
{
  /* Store out handler to call */
  swexn_handler_t eip = curr_thr->swexn.eip;
  void *esp3 = curr_thr->swexn.esp3;
  void *arg = curr_thr->swexn.arg;

  /* Deregister old handler */
  deregister(&curr_thr->swexn);

  /* Copy the execution state onto the exception stack */
  esp3 = (char *)(esp3) - sizeof(ureg_t) - sizeof(unsigned int);
  memcpy(esp3, state, sizeof(ureg_t));
  void *addr = esp3;

  /* Craft contents of exception stack */
  PUSH(esp3, addr);     /* Address of executation state on exception stack */
  PUSH(esp3, arg);      /* Opaque void * arg */
  PUSH(esp3, 0);        /* Dummy return address */

  /* Run the handler in user mode */
  half_dispatch(eip, esp3);

  return;
}
    
/** @brief Deregister a software exception handler.
 *
 *  We use NULL to denote a deregistered handler.
 *
 *  @param swexn Address of handler to deregister.
 *
 *  @return Void.
 */
void deregister(swexn_t *swexn)
{
  swexn->esp3 = NULL;
  swexn->eip = NULL;
  swexn->arg = NULL;

  return;
}

/** @brief Validate register values.
 *
 *  @param regs Registers to validate.
 *
 *  @return -1 if invalid, else 0.
 **/
int validate_regs(ureg_t *regs)
{
  /* Validate data and segments */
  if((regs->ds != SEGSEL_USER_DS) || (regs->es != SEGSEL_USER_DS) ||
     (regs->fs != SEGSEL_USER_DS) || (regs->gs != SEGSEL_USER_DS) ||
     (regs->ss != SEGSEL_USER_DS)){
    return -1;
  }

  /* Validate code segment */
  if(regs->cs != SEGSEL_USER_CS)
    return -1;

  /* Validate EFLAGS */
  if((regs->eflags & EFL_SET) != EFL_SET)
    return -1;  

  if((regs->eflags & EFL_UNSET) != 0)
    return -1;  

  /* Validate EIP and ESP */
  if((validate_sp((void *)regs->esp)) ||  (validate_pc((void *)regs->eip)))
   return -1;

  return 0;
}

/** @brief Validate stack pointer.
 *
 *  @Bug there's nothing to prevent us from validating the stack and then
 *  another thread freeing it
 *
 *  @param sp Stack pointer to validate.
 *
 *  @return -1 if invalid, else 0.
 **/
int validate_sp(void *sp)
{
  unsigned int attrs;

  mutex_lock(&curr_tsk->lock);

  /* Acquire the region's attributes */
  assert(!vm_get_attrs(&curr_tsk->vmi, sp, &attrs));

  mutex_unlock(&curr_tsk->lock);

  /* Stack must be writable and accessible in user mode */
  if((attrs & VM_ATTR_RDWR) && (attrs & VM_ATTR_USER))
    return 0;
  else
    return -1;
}

/** @brief Validate program counter.
 *
 *  @param pc Program counter to validate.
 *
 *  @return -1 if invalid, else 0.
 **/
int validate_pc(void *pc)
{
  unsigned int attrs;

  mutex_lock(&curr_tsk->lock);

  /* Acquire the region's attributes */
  assert(!vm_get_attrs(&curr_tsk->vmi, pc, &attrs));

  mutex_unlock(&curr_tsk->lock);

  /* Code must be accessible in user mode and not writeable */
  if((!(attrs & VM_ATTR_RDWR)) && (attrs & VM_ATTR_USER))
    return 0;
  else
    return -1;
}

/** @brief Safely copy data from user-space.
 *
 *  We malloc in order to avoid imposing an artificial restriction on the number
 *  of arguments a user can pass to exec or a filename.  If there is not enough
 *  space, then malloc will fail.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) source pointer.
 *  @param bytes The number of bytes to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_from_user(char **dst, const char *src, size_t bytes)
{
  mutex_lock(&curr_tsk->lock);

  if (!vm_find(&curr_tsk->vmi, (void *)src)) {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  *dst = malloc(bytes);
  if (!*dst) return -1;

  memcpy(*dst, src, bytes);

  mutex_unlock(&curr_tsk->lock);
  return 0;
}

/** @brief Statically safely copy data from user-space.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) source pointer.
 *  @param bytes The number of bytes to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_from_user_static(void *dst, void *src, size_t bytes)
{
  mutex_lock(&curr_tsk->lock);

  if (!vm_find(&curr_tsk->vmi, src)) {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  memcpy(dst, src, bytes);

  mutex_unlock(&curr_tsk->lock);
  return 0;
}
/** @brief Safely copy data to user-space.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) source pointer.
 *  @param bytes The number of bytes to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_to_user(char *dst, const char *src, size_t bytes)
{
  unsigned int attrs;

  mutex_lock(&curr_tsk->lock);

  if (vm_get_attrs(&curr_tsk->vmi, dst, &attrs)
      || !(attrs & VM_ATTR_RDWR))
  {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  memcpy(dst, src, bytes);

  mutex_unlock(&curr_tsk->lock);
  return 0;
}

/** @brief Safely copy a string from user-space.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) string pointer.
 *
 *  @return The length of the string on success; a negative integer error
 *  code on failure.
 **/
int copy_str_from_user(char **dst, const char *src)
{
  size_t len;

  mutex_lock(&curr_tsk->lock);

  if (!vm_find(&curr_tsk->vmi, (void *)src)) {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }

  len = strlen(src) + 1;
  *dst = malloc(len);
  if (!*dst) return -1;

  memcpy(*dst, src, len);

  mutex_unlock(&curr_tsk->lock);
  return len;
}

/** @brief Safely copy a NULL-terminated character array from user-space.
 *
 *  @param dst The destionation array.
 *  @param src The (user-space) array.
 *
 *  @return The number of array entries on success; a negative integer
 *  error code on failure.
 **/
int copy_argv_from_user(char **dst[], char *src[])
{
  int argc;
  char **argv;
  int i, j;

  /* Safely calculate arg vector length */
  mutex_lock(&curr_tsk->lock);
  if (!vm_find(&curr_tsk->vmi, (void *)src)) {
    mutex_unlock(&curr_tsk->lock);
    return -1;
  }
  for (argc = 0; src[argc] != NULL; ++argc) continue;
  mutex_unlock(&curr_tsk->lock);

  /* Allocate kernel mem for kernel-side arg vector */
  argv = malloc((argc + 1) * sizeof(char *));
  if (!argv) return -1;

  /* Copy each arg from user-space */
  for (i = 0; i < argc; ++i)
  {
    if (copy_str_from_user(&argv[i], src[i]) < 0)
    {
      for (j = 0; j < i; ++j) free(argv[j]);
      free(argv);
      return -1;
    }
  }

  /* Null terminate */
  argv[argc] = NULL;

  /* Return kernel argv and argc */
  *dst = argv;
  return argc;
}

/** @brief Installs our system calls.
 *
 *  @return Void.
 **/
void install_sys_handlers(void)
{
  install_trap_gate(FORK_INT, asm_sys_fork, IDT_USER_DPL);
  install_trap_gate(THREAD_FORK_INT, asm_sys_thread_fork, IDT_USER_DPL);
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

