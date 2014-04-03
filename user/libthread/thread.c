/** @file thr_init.c
 *
 *  @brief Implements the thread library API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */
#include <simics.h>

#include <thr_internals.h>

#include <assert.h>
#include <atomic.h>
#include <malloc.h>
#include <mutex.h>
#include <stack.h>
#include <syscall.h>
#include <tcb.h>


/* Prototypes for helper functions */
void thread_exn_handler(void *arg, ureg_t *ureg);


/*************************************************************************
 *  API Functions
 *************************************************************************/

/** @brief Initialize the thread library.
 *
 *  Store user requested (page-aligned) thread stack size in a global.  The
 *  page allignement is necesary because new_pages(...) expects a page
 *  aligned length argument. thr_internals.h also has a PAGE_CEILING which
 *  may be preferable in this situation, however it was causing compilation
 *  warnings.
 *
 *  @param size The requested thread stack size.
 *
 *  @return -1 if size is invalid (i.e. 0) otherwise 0
 */
int thr_init(unsigned int size)
{
  /* Store stack size */
  if(size == 0) return -1;
  thread_stack_size = PAGE_CEILING(size);

  lprintf("installing swexn");
  /* Install thread exception handler */
  swexn(exn_stack + EXN_STACK_SIZE, thread_exn_handler, NULL, NULL);


  lprintf("thrlist init");
  /* Init thread list and the caller's TCB */
  thrlist_init(&main_tcb);

  return 0;
}

/** @brief Create a new thread.
 *
 *  Allocate a TCB and stack for the child thread and perform the actual
 *  forking.
 *
 *  @param func A pointer to the function child thread should run.
 *  @param arg Arguments for func.
 *
 *  @return The child's TID.
 */
int thr_create(void *(*func)(void *),void *arg)
{
  void *base, *sp;
  int tid;
  tcb_s *tcb;

  /* Allocate the thread's stack */
	base = stack_alloc();
  if (base == NULL) return -1;

  /*****************************************
   *  TCB setup
   *****************************************/

  /* Allocate a TCB */
	tcb = malloc(sizeof(tcb_s));
  if (tcb == NULL) {
    assert(remove_pages(base) == 0);
    return -1;
  }

  /* Add the TCV to the thread list */
  tcb_init(tcb, -1, base);
  thrlist_add(tcb);

  /*****************************************
   *  Stack setup
   *****************************************/

  /* Calculate the top of the stack */
  sp = STACK_HIGH(base);

  /* TCB pointer for quick access */
  PUSH(sp, tcb);

  /* Setup stack for call to child_init(...) */
  PUSH(sp, arg);
  PUSH(sp, func);
  PUSH(sp, tcb);

  /*****************************************
   *  Forking
   *****************************************/

  /* Fork the new thread.
   * (only the parent returns)
   */
  tid = thread_fork(sp);
  assert(tid != 0);

  /* Clean-up after failed fork */
	if (tid < 0) {
		assert(remove_pages(base) == 0);
    free(tcb);
    return -1;
	}

  /* Finalize after successfull fork */
	else {
    tcb->tid = tid;
    compare_and_swap((volatile unsigned int *)(&tcb->state),
                     THR_NASCENT, THR_ACTIVE);
	}

	return tid;
}

/** @brief Terminate a thread.
 *
 *  To exit, we set the caller's exit status, wake up the joiner (if there
 *  is one), free the caller's stack for reuse, then vanish.  The TCB
 *  remains; the joiner will free it after reaping the exit status.
 *
 *  @param status The exit status.
 *
 *  @return Does not return.
 */
void thr_exit(void *status)
{
  tcb_s *tcb;
  void *stack, **listp;

  /* Find your own TCB */
  tcb = thrlist_owntcb();
  assert(tcb);
  tcb_lock(tcb);

  /* Set exit state and status */
  tcb->state = THR_EXITED;
  tcb->status = status;

  /* Grab the stack (for freeing later) */
  stack = tcb->stack;

  /* Unlock and (maybe) run the joiner */
  if (tcb->joinp) {
    tcb_unlock(tcb);
    tcb->joinp->reject = gettid();
    make_runnable(tcb->joinp->tid);
  }
  else tcb_unlock(tcb);

  /* Free the stack and vanish */
  listp = stack_create_entry();
  remove_and_vanish(stack, listp);
  return;
}

/** @brief Reap a thread.
 *
 *  If the joinee has not exited, deschedule the caller until s/he exits.
 *  Afer the joinee has exited, reap the his/her exit status and clean up
 *  their TCB.
 *
 *  @param tid The joinee's TID.
 *  @param statusp A pointer to which to write the joinee's exit status.
 *
 *  @return Does not return.
 */
int thr_join(int tid, void **statusp)
{
  tcb_s *tcb, *me;

  /* Find and lock joinee's TCB */
  tcb = thrlist_findtcb(tid);
  tcb_lock(tcb);

  /* Check error conditions */
  if (tcb == NULL) return -1;
  else if (tcb->joinp != NULL) return -1;

  /* Thread hasn't exited yet */
  if (tcb->state != THR_EXITED) {
    //me = thrlist_owntcb();
    me = thrlist_owntcb();
    assert(me);
    tcb->joinp = me;

    /* Unlock and block (and lock) */
    me->reject = 0;
    tcb_unlock(tcb);
    while (me->reject != tcb->tid)
      deschedule(&me->reject);
    tcb_lock(tcb);
  }

  /* Take what you came for */
  if (statusp) *statusp = tcb->status;

  /* Clean up the TCB */
  thrlist_del(tcb);
  tcb_unlock(tcb);
  tcb_final(tcb);
  free(tcb);

  return 0;
}

/** @brief Return the caller's TID.
 *
 *  For speed, we lookup the TID using the TCB pointer atop each thread's
 *  stack.
 *
 *  @return The caller's TID.
 */
int thr_getid(void)
{
  tcb_s *tcb;
  tcb = thrlist_owntcb();
  return tcb->tid;
}

/** @brief Yield to the specified thread.
 *
 *  @param tid The TID of the thread we'll yield to.
 *
 *  @return 0 on success; a negative integer error code otherwise.
 */
int thr_yield(int tid)
{
  return yield(tid);
}


/*************************************************************************
 *  Helper Functions
 *************************************************************************/

/** @brief Entry/bottom function for new threads.
 *
 *  Save the child's TID and call the thread function.
 *
 *  @param tcb The child's TCB.
 *  @param func The function the child is to execute.
 *  @param arg Arguments for func.
 *
 *  @return Does not return.
 */
void child_init(tcb_s *tcb, void *(*func)(void *), void *arg)
{
    void *status;

    /* Finalize TCB setup
     */
    if (tcb->state == THR_NASCENT) tcb->tid = gettid();
    compare_and_swap((volatile unsigned int *)(&tcb->state),
                     THR_NASCENT, THR_ACTIVE);

    /* Call thread function and exit */
    status = func(arg);
    thr_exit(status);

    /* thr_exit(...) does not return */
}

#include <simics.h>

/** @brief Handle exceptions for multithreaded programs.
 *
 *  If any thread encounters and error, we kill the entire process.  We
 *  cannot be sure that the faulting thread isn't holding a lock, hasn't
 *  mangled some important data structure, or otherwise left the process in
 *  an unsafe state.
 *
 *  @param arg Unused.
 *  @param ureg User-space register values at the time of the exception.
 *
 *  @return Void.
 **/
void thread_exn_handler(void *arg, ureg_t *ureg)
{
  lprintf("Encountered fatal software exception %d", ureg->cause);
  lprintf(" ");

  /* General purpose registers: callee, then caller */
  lprintf("eax: 0x%08X   ecx: 0x%08X   edx: 0x%08X",
          ureg->eax, ureg->ecx, ureg->edx);
  lprintf("ebx: 0x%08X   esi: 0x%08X   edi: 0x%08X",
          ureg->ebx, ureg->esi, ureg->edi);
  lprintf(" ");

  /* Stack register and the program counter */
  lprintf("esp: 0x%08X   ebp: 0x%08X   eip: 0x%08X",
          ureg->esp, ureg->ebp, ureg->eip);
  lprintf(" ");

  /* Segment registers */
  lprintf(" ss: 0x%08X    cs: 0x%08X    ds: 0x%08X",
          ureg->ss, ureg->cs, ureg->ds);
  lprintf(" es: 0x%08X    fs: 0x%08X    gs: 0x%08X",
          ureg->es, ureg->fs, ureg->gs);
  lprintf(" ");

  /* EFLAGS */
  lprintf("eflags: 0x%08X", ureg->eflags);
  lprintf(" ");

  /* Kill the program */
  panic(NULL);
  return;
}

