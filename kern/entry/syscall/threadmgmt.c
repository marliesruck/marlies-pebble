/** @file threadmgmt.c
 *
 *  @brief Implements our thread management system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sched.h>
#include <thread.h>
#include <timer.h>


/*************************************************************************
 *  Thread management system calls
 *************************************************************************/

/** @brief Retrieve the TID of the invoking thread.
 *
 *  @return The TID of the invoking thread.
 **/
int sys_gettid(void)
{
  return curr_thr->tid;
}

/** @brief Defer execution of the invoking thread in favor of another.
 *
 *  The invoking thread's execution will be suspended until the thread
 *  specified by tid runs.  If the no thread with TID tid exists or it is
 *  not currently runnable, or if the invoking thread specifies it's own
 *  TID, sys_yield(...) returns an error code.  If tid = -1, the invoking
 *  thread is descheduled until each currently runnable thread runs.
 *
 *  @param tid The TID of the thread to yield to.
 *
 *  @return 0 on success; or a negative integer error code on failure.
 **/
int sys_yield(int tid)
{
  /* Yield to anyone */
  if(tid == -1){
    schedule();
    return 0;
  }

  /* Ensure the favored thread is runnable */
  return sched_find(tid);
}

/** @brief Deschedule the invoking thread.
 *
 *  This function checks the value pointed at by reject and, if it is zero,
 *  deschedules the invoking thread until a corresponding call to
 *  make_runnable(...).  If the reject pointer is invalid, an error code is
 *  returned.  The process of checking the reject pointer and descheduling
 *  is atomic with respect to calls to make_runnable(...) on the invoking
 *  thread.
 *
 *  @param reject A pointer to the reject integer.
 *
 *  @return 0 on success; or a negative integer error code on failure.
 **/
int sys_deschedule(int *reject)
{
  unsigned int attrs;

  mutex_lock(&curr_thr->lock);

  /* Make sure the reject is valid
   * can't use copy_from_user(...); already have the task lock
   */
  if (vm_get_attrs(&curr_tsk->vmi, reject, &attrs)
      || !(attrs & VM_ATTR_RDWR))
  {
    mutex_unlock(&curr_thr->lock);
    return -1;
  }

  /* Check reject value */
  if (*reject) {
    mutex_unlock(&curr_thr->lock);
    return 0;
  }

  curr_thr->desched = THR_DESCHED;
  mutex_unlock_and_block(&curr_thr->lock);

  return 0;
}

/** @brief Makes the thread specified by tid runnable.
 *
 *  This function attempts to make a blocked thread runnable.  If the
 *  thread specified by TID does not exist, is currently runnable, or is
 *  currently blocked to do something other than a call to
 *  sys_deschedule(...), sys_make_runnable(...) returns an error code.
 *
 *  @param tid The TID of the thread to make runnable.
 *
 *  @return 0 on success; or a negative integer error code on failure.
 **/
int sys_make_runnable(int tid)
{
  thread_t *thr;

  /* Find and lock the target */
  thr = thrlist_find(tid);
  mutex_lock(&thr->lock);

  /* The target must exist and be runnable */
  if (!thr || thr->state == THR_RUNNABLE || thr->desched != THR_DESCHED)
  {
    mutex_unlock(&thr->lock);
    return -1;
  }

  /* Unblock the target */
  thr->desched = THR_NOT_DESCHED;
  sched_unblock(thr);
  mutex_unlock(&thr->lock);
  return 0;
}

/** @brief Retieve the number of ticks since boot.
 *
 *  @return The number of ticks since system boot.
 **/
unsigned int sys_get_ticks(void)
{
  return tmr_get_ticks();
}

/** @brief Deschedules the calling thread for ticks timer interrupts.
 *
 *  If ticks is 0, sys_sleep(...) returns immediately.  If ticks is
 *  negative, an error code is returned.  Otherwise the invoking thread is
 *  put to sleep until ticks timer interrupts have occurred.
 **/
int sys_sleep(int ticks)
{
  unsigned int time;

  if (ticks == 0) return 0;
  if (ticks < 0) return -1;

  time = tmr_get_ticks();
  go_to_sleep(curr_thr, time + ticks);

  return 0;
}

