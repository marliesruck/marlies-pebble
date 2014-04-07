/** @file faults.c
 *
 *  @brief Implements our various fault handlers.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include "fault_wrappers.h"

/* Pebbles specific includes */
#include <idt.h>
#include <sched.h>
#include <sc_utils.h>
#include <ureg.h>

/* Libc specific includes */
#include <assert.h>
#include <string.h>   /* FOR DEBUGGING */

/* x86 specific includes */
#include <x86/cr.h>
#include <x86/idt.h>

/* TESTS THAT SHOULD FAIL */
char *fail[] = {
  "remove_pages_test2",
  "wild_test1",
  "swexn_stands_for_swextensible",
  "swexn_uninstall_test",
};

/* @brief Pointer to a kernel handler.
 *
 * Let the kernel attempt to silently handle the fault. If it fails, the kernel
 * can populate the ureg struct cause and addr fields accordingly, then return 
 * and call the user's handler if installed.  
 *
 * @param ureg Current execution state.
 *
 * @ return 0 if the kernel successfully handles, else -1. 
 **/
typedef int (*handler)(ureg_t *ureg);

/** @brief Generic function called by fault wrappers
 *
 *  This fault wrapper is called from all faults that may have a software
 *  exception handler installed.  The fault wrapper first allows the kernel to
 *  attempt to silently handle the fault.  If this fails, the user software
 *  exception handler is called, and if that fails then the thread is killed.
 *
 *  @param f Pointer to kernel fault handler.
 *
 *  @return Void.
 **/
void fault_wrapper(handler f)
{
  ureg_t *ureg;

  /* Retrieve execution state */
  ureg = (ureg_t *)((char *)(get_ebp()) + sizeof(unsigned));

  /* Let the kernel try to silently handle */
  if(f(ureg) < 0){

    /* Call the user defined exception handler */
    if(curr_thr->swexn.eip){

      /* Craft contents of exception stack and call handler */
      init_exn_stack(ureg);
    }

    /* TODO: ELIMINATE!
     * Avoid false negatives */
    int i;
    int should_fail = 0;
    int num_tests = sizeof(fail)/sizeof(char*);
    for(i = 0; i < num_tests; i++){
      /* This thread should be killed */
      if(!strcmp(curr_tsk->execname, fail[i]))
        should_fail = 1;
    }
    if(!should_fail)
      MAGIC_BREAK;

    /* You were killed by the kernel */
    slaughter();
  }
  return;
}

/** @brief Installs our fault handlers.
 *
 *  All our fault handlers use trap gates, except for page faults, because we
 *  want to avoid a nested page fault that would clobber cr3 for the interrupted
 *  handler.  
 *
 *  @return Void.
 **/
void install_fault_handlers(void)
{
  install_trap_gate(IDT_DE, asm_int_divzero, IDT_KERN_DPL);
  install_trap_gate(IDT_DB, asm_int_debug, IDT_KERN_DPL);
  install_trap_gate(IDT_NMI, asm_int_nmi, IDT_KERN_DPL);
  install_trap_gate(IDT_BP, asm_int_breakpoint, IDT_KERN_DPL);
  install_trap_gate(IDT_OF, asm_int_overflow, IDT_KERN_DPL);
  install_trap_gate(IDT_BR, asm_int_bound, IDT_KERN_DPL);
  install_trap_gate(IDT_UD, asm_int_undef_opcode, IDT_KERN_DPL);
  install_trap_gate(IDT_NM, asm_int_device_unavail, IDT_KERN_DPL);
  install_trap_gate(IDT_DF, asm_int_double_fault, IDT_KERN_DPL);
  install_trap_gate(IDT_CSO, asm_int_cso, IDT_KERN_DPL);
  install_trap_gate(IDT_TS, asm_int_tss, IDT_KERN_DPL);
  install_trap_gate(IDT_NP, asm_int_seg_not_present, IDT_KERN_DPL);
  install_trap_gate(IDT_SS, asm_int_stack_seg, IDT_KERN_DPL);
  install_trap_gate(IDT_GP, asm_int_gen_prot, IDT_KERN_DPL);
  install_trap_gate(IDT_MF, asm_int_float, IDT_KERN_DPL);
  install_trap_gate(IDT_AC, asm_int_align, IDT_KERN_DPL);
  install_trap_gate(IDT_MC, asm_int_machine_check, IDT_KERN_DPL);
  install_trap_gate(IDT_XF, asm_int_simd, IDT_KERN_DPL);
  install_interrupt_gate(IDT_PF, asm_int_page_fault, IDT_KERN_DPL);

  return;
}

/** @brief Handles division by zero.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_divzero(ureg_t *ureg)
{
  lprintf("Error: Division by zero!");

  ureg->cause = SWEXN_CAUSE_DIVIDE;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles debug interrupts.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_debug(ureg_t *ureg)
{
  lprintf("Alert: Got debug interrupt...");

  ureg->cause = SWEXN_CAUSE_DEBUG;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles non-maskable interrupts.
 *
 *  @return Void.
 **/
void int_nmi(void)
{
  lprintf("Error: Non-maskable interrupt!");

  slaughter();

  return;
}

/** @brief Handles breakpoint interrupts.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_breakpoint(ureg_t *ureg)
{
  lprintf("Alert: Encountered breakpoint (INT 3)!");

  ureg->cause = SWEXN_CAUSE_BREAKPOINT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles overflow exceptions.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_overflow(ureg_t *ureg)
{
  lprintf("Error: Overflow (INTO)!");

  ureg->cause = SWEXN_CAUSE_OVERFLOW;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles out of bounds exceptions.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_bound(ureg_t *ureg)
{
  lprintf("Error: Range exceeded (BOUND)!");

  ureg->cause = SWEXN_CAUSE_BOUNDCHECK;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles invalid instructions.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_undef_opcode(ureg_t *ureg)
{
  lprintf("Error: Invalid instruction!");

  ureg->cause = SWEXN_CAUSE_OPCODE;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles unavailable devices.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_device_unavail(ureg_t *ureg)
{
  lprintf("Error: Device not available!");

  ureg->cause = SWEXN_CAUSE_NOFPU;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles double faults.
 *
 *  @return Void.
 **/
void int_double_fault(void)
{
  lprintf("Error: Double fault!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles coprocessor segment overruns.
 *
 *  @return Void.
 **/
void int_cso(void)
{
  lprintf("Error: Coprocessor segment overrun!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles invalid task segment selectors.
 *
 *  @return Void.
 **/
void int_tss(void)
{
  lprintf("Error: Invalid task segment selector!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles non-present segments.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_seg_not_present(ureg_t *ureg)
{
  lprintf("Error: segment not present!");

  ureg->cause = SWEXN_CAUSE_SEGFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles stack segment faults.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_stack_seg(ureg_t *ureg)
{
  lprintf("Error: stack segmentation fault!");

  ureg->cause = SWEXN_CAUSE_STACKFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles general protection faults.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_gen_prot(ureg_t *ureg)
{
  lprintf("Error: general protection fault!");

  ureg->cause = SWEXN_CAUSE_PROTFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles page faults.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_page_fault(ureg_t *ureg)
{
  void *cr2;
  int retval;

  /* Grab the faulting cr2ess and page info */
  cr2 = (void *)get_cr2();

  /* Try to handle the fault */
  if ((retval = pg_page_fault_handler(cr2)))
  {
    lprintf("Error:\nPage fault handler returned %d\nFaulting address %p\n"
            "Faulting instruction: 0x%x\nFaulting task: %s", retval, cr2, 
             ureg->eip,curr_tsk->execname);

    ureg->cause = SWEXN_CAUSE_PAGEFAULT;
    ureg->cr2 = (unsigned int)cr2;

    return -1;
  }
  return 0;
}

/** @brief Handles floating point exceptions.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_float(ureg_t *ureg)
{
  lprintf("Error: Floating point exception!");

  ureg->cause = SWEXN_CAUSE_FPUFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles (failed?) alignment checks.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_align(ureg_t *ureg)
{
  lprintf("Error: Alignment check!");

  ureg->cause = SWEXN_CAUSE_ALIGNFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles machine checks.
 *
 *  @return Void.
 **/
void int_machine_check(void)
{
  lprintf("Error: Machine check!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles SIMD exceptions.
 *
 *  @return 0 if the kernel handles the fault, else -1.
 **/
int int_simd(ureg_t *ureg)
{
  lprintf("Error: SIMD floating point exception!");

  ureg->cause = SWEXN_CAUSE_SIMDFAULT;
  ureg->cr2 = 0;

  return -1;
}

/** @brief Handles pretty much anything else.
 *
 *  @return Void.
 **/
void int_generic(void)
{
  lprintf("Error: Got a fault!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

