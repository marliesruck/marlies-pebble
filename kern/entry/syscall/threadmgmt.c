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

int sys_gettid(void)
{
  return curr_thr->tid;
}

int sys_yield(int pid)
{
  /* Yield to anyone */
  if(pid == -1){
    schedule();
    return 0;
  }

  /* Ensure the favored thread is runnable */
  return sched_find(pid);
}

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

  sched_do_and_block(curr_thr, (sched_do_fn) mutex_unlock_raw,
                     &curr_thr->lock);

  return 0;
}

int sys_make_runnable(int tid)
{
  thread_t *thr;
  int ret;

  /* Find and lock the target */
  thr = thrlist_find(tid);
  mutex_lock(&thr->lock);

  if (!thr || thr->state == THR_RUNNABLE) {
    mutex_unlock(&thr->lock);
    return -1;
  }

  ret = sched_unblock(thr);
  mutex_unlock(&thr->lock);

  return ret;
}

unsigned int sys_get_ticks(void)
{
  return tmr_get_ticks();
}

int sys_sleep(int ticks)
{
  unsigned int time;

  if (ticks < 0) return -1;

  time = tmr_get_ticks();
  go_to_sleep(curr_thr, time + ticks);

  return 0;
}

