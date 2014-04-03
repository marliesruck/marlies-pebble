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

/* x86 specific includes */
#include <x86/idt.h>

/** @brief Installs our fault handlers.
 *
 *  @return Void.
 **/
void install_fault_handlers(void)
{
  install_interrupt_gate(IDT_DE, asm_int_divzero, IDT_KERN_DPL);
  install_interrupt_gate(IDT_DB, asm_int_debug, IDT_KERN_DPL);
  install_interrupt_gate(IDT_NMI, asm_int_nmi, IDT_KERN_DPL);
  install_interrupt_gate(IDT_BP, asm_int_breakpoint, IDT_KERN_DPL);
  install_interrupt_gate(IDT_OF, asm_int_overflow, IDT_KERN_DPL);
  install_interrupt_gate(IDT_BR, asm_int_bound, IDT_KERN_DPL);
  install_interrupt_gate(IDT_UD, asm_int_undef_opcode, IDT_KERN_DPL);
  install_interrupt_gate(IDT_NM, asm_int_device_unavail, IDT_KERN_DPL);
  install_interrupt_gate(IDT_DF, asm_int_double_fault, IDT_KERN_DPL);
  install_interrupt_gate(IDT_CSO, asm_int_cso, IDT_KERN_DPL);
  install_interrupt_gate(IDT_TS, asm_int_tss, IDT_KERN_DPL);
  install_interrupt_gate(IDT_NP, asm_int_seg_not_present, IDT_KERN_DPL);
  install_interrupt_gate(IDT_SS, asm_int_stack_seg, IDT_KERN_DPL);
  install_interrupt_gate(IDT_GP, asm_int_gen_prot, IDT_KERN_DPL);
  install_interrupt_gate(IDT_PF, asm_int_page_fault, IDT_KERN_DPL);
  install_interrupt_gate(IDT_MF, asm_int_float, IDT_KERN_DPL);
  install_interrupt_gate(IDT_AC, asm_int_align, IDT_KERN_DPL);
  install_interrupt_gate(IDT_MC, asm_int_machine_check, IDT_KERN_DPL);
  install_interrupt_gate(IDT_XF, asm_int_simd, IDT_KERN_DPL);

  return;
}

/** @brief Handles division by zero.
 *
 *  @return Void.
 **/
void int_divzero(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_DIVIDE, NULL);
  }

  lprintf("Error: Division by zero!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles debug interrupts.
 *
 *  @return Void.
 **/
void int_debug(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_DEBUG, NULL);
  }

  lprintf("Alert: Got debug interrupt...");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles non-maskable interrupts.
 *
 *  @return Void.
 **/
void int_nmi(void)
{
  lprintf("Error: Non-maskable interrupt!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles breakpoint interrupts.
 *
 *  @return Void.
 **/
void int_breakpoint(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_BREAKPOINT, NULL);
  }

  lprintf("Alert: Encountered breakpoint (INT 3)!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles overflow exceptions.
 *
 *  @return Void.
 **/
void int_overflow(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_OVERFLOW, NULL);
  }

  lprintf("Error: Overflow (INTO)!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles out of bounds exceptions.
 *
 *  @return Void.
 **/
void int_bound(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_BOUNDCHECK, NULL);
  }

  lprintf("Error: Range exceeded (BOUND)!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles invalid instructions.
 *
 *  @return Void.
 **/
void int_undef_opcode(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_OPCODE, NULL);
  }

  lprintf("Error: Invalid instruction!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles unavailable devices.
 *
 *  @return Void.
 **/
void int_device_unavail(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_NOFPU, NULL);
  }
  lprintf("Error: Device not available!");

  /* You were killed by the kernel */
  slaughter();

  return;
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
 *  @return Void.
 **/
void int_seg_not_present(void)
{

  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_SEGFAULT, NULL);
  }

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles stack segment faults.
 *
 *  @return Void.
 **/
void int_stack_seg(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){


    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_STACKFAULT, NULL);
  }

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles general protection faults.
 *
 *  @return Void.
 **/
void int_gen_prot(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_PROTFAULT, NULL);
  }

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles page faults.
 *
 *  @return Void.
 **/
/* Pebbles specific includes */
#include <sched.h>
/* x86 specific includes */
#include <x86/cr.h>
/* Libc includes */
#include <string.h>
void int_page_fault(void)
{
  void *addr;
  ureg_t *state;

  /* Grab the faulting address and page info */
  addr = (void *)get_cr2();

  /* Try to handle the fault */
  if (pg_page_fault_handler(addr))
  {
    /* A software exception handler was installed by the user */
    if(curr->swexn.eip){

      /* Retrieve execution state */
      state = (ureg_t *)(get_ebp());

      /* Craft contents of exception stack and call handler */
      init_exn_stack(state, SWEXN_CAUSE_PAGEFAULT, addr);
    }
    else{
      lprintf("Error: Page fault on table-less address %p by thread: %d", 
              addr, curr->tid);

      /* You were killed by the kernel */
      slaughter();
    }
  }

  return;
}

/** @brief Handles floating point exceptions.
 *
 *  @return Void.
 **/
void int_float(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_FPUFAULT, NULL);
  }
  lprintf("Error: Floating point exception!");

  /* You were killed by the kernel */
  slaughter();

  return;
}

/** @brief Handles (failed?) alignment checks.
 *
 *  @return Void.
 **/
void int_align(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_ALIGNFAULT, NULL);
  }
  lprintf("Error: Alignment check!");

  /* You were killed by the kernel */
  slaughter();

  return;
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
 *  @return Void.
 **/
void int_simd(void)
{
  ureg_t *state;

  /* A software exception handler was installed by the user */
  if(curr->swexn.eip){

    /* Retrieve execution state */
    state = (ureg_t *)(get_ebp());

    /* Craft contents of exception stack and call handler */
    init_exn_stack(state, SWEXN_CAUSE_SIMDFAULT, NULL);
  }
  lprintf("Error: SIMD floating point exception!");

  /* You were killed by the kernel */
  slaughter();

  return;
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

