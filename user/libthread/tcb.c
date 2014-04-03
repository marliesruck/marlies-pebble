/** @file tcb.c
 *
 *  @brief Implements functinality related to thread control blocks (TCBs)
 *  and the thread list.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */

#include <thr_internals.h>

#include <assert.h>
#include <malloc.h>
#include <stack.h>
#include <string.h>
#include <syscall.h>
#include <tcb.h>


/*************************************************************************
 *  TCB Functions
 *************************************************************************/

/** @brief Initialize a TCB.
 *
 *  @param tcb The TCB to initialize.
 *  @param tid The TID to store in the TCB.
 *  @param stack The base of the stack to store in th TCB.
 *
 *  @return Void.
 **/
void tcb_init(tcb_s *tcb, int tid, void *stack)
{
  mutex_init(&tcb->lock);
  tcb->state = THR_NASCENT;
	tcb->tid = tid;
	tcb->reject = 0; 
  tcb->stack = stack;
	tcb->status = NULL;
	tcb->joinp = NULL;

  return;
}

/** @brief Finalize a TCB.
 *
 *  @param tcb The TCB to finalize.
 *
 *  @return Void.
 **/
void tcb_final(tcb_s *tcb)
{
  mutex_destroy(&tcb->lock);
  memset(tcb, 0, sizeof(tcb_s));
  return;
}

/** @brief Attempt to gain exclusive access to a TCB.
 *
 *  @param tcb The TCB to lock.
 *
 *  @return Void.
 **/
inline void tcb_lock(tcb_s *tcb)
{
  mutex_lock(&tcb->lock);
  return;
}

/** @brief Relinquish exclusive access to a TCB.
 *
 *  @param tcb The TCB to unlock.
 *
 *  @return Void.
 **/
inline void tcb_unlock(tcb_s *tcb)
{
  mutex_unlock(&tcb->lock);
  return;
}


/*************************************************************************
 *  Thread List Functions
 *************************************************************************/

/** @brief Initialize the thread list and a TCB for the caller.
 *
 *  @param tcb The TCB of the root thread.
 *
 *  @return Void.
 **/
void thrlist_init(tcb_s *tcb)
{
  /* Init the thread list */
  mutex_init(&thread_list.lock);
  cll_init_list(&thread_list.list); 


  /* Give the caller a TCB */
  tcb_init(tcb, gettid(), NULL);
  thrlist_add(tcb);

  return;
}

/** @brief Add a TCB to the thread lust.
 *
 *  @param tcb The TCB to add.
 *
 *  @return Void.
 **/
void thrlist_add(tcb_s *tcb)
{
  cll_node *n;

  /* Allocate a node for the new TCB */
	n = malloc(sizeof(cll_node));
  cll_init_node(n, (void *)tcb);

  /* Lock, insert, unlock */
  mutex_lock(&thread_list.lock);
  cll_insert(thread_list.list.next, n);
  mutex_unlock(&thread_list.lock);

  return;
}

/** @brief Delete a TCB to the thread list.
 *
 *  @param tcb The TCB to delete.
 *
 *  @return Void.
 **/
void thrlist_del(tcb_s *tcb)
{
  cll_node *n;

  mutex_lock(&thread_list.lock);

  /* Find our TCB in the thread list */
  cll_foreach(&thread_list.list, n)
    if (cll_entry(tcb_s *,n) == tcb) break;
  assert(cll_entry(tcb_s *,n) == tcb);

  /* Extract and free it */
  assert(cll_extract(&thread_list.list, n));
  free(n);

  mutex_unlock(&thread_list.lock);
  return;
}

/** @brief Search the thread list by TID for a specific TCB.
 *
 *  @param tid The TID of the TCB to look for.
 *
 *  @return A pointer to the TCB, or NULL if not found.
 **/
tcb_s *thrlist_findtcb(int tid)
{
  cll_node *n;
  tcb_s *tcb;

  /* Callers *should* use thrlist_owntcb for this instead */
  if (tid == -1) tid = gettid();

  mutex_lock(&thread_list.lock);

  /* Iteratively search the thread list */
  cll_foreach(&thread_list.list, n) {
    tcb = cll_entry(tcb_s *,n);
    if (tcb->tid == tid) {
      mutex_unlock(&thread_list.lock);
      return tcb;
    }
  }

  mutex_unlock(&thread_list.lock);
  return NULL;
}

/** @brief Retrieve the caller's TCB from their stack.
 *
 *  Note that this function is not strictly thread list-related.  However,
 *  it is here because it is analagous to thrlist_findtcb(...).
 *
 *  @param tid The TID of the TCB to look for.
 *
 *  @return A pointer to the TCB, or NULL if not found.
 **/
tcb_s *thrlist_owntcb(void){

  esp_t sp = get_esp();
  if(sp >= (esp_t)(sp_low))
    return &main_tcb;

  esp_t *sp_high = (esp_t *)(PAGE_CEILING(sp));
  DECREMENT(sp_high);

  tcb_s *tcb = *(tcb_s **)(sp_high);
  
  assert(tcb->tid == gettid());

  return tcb;
}

/** @brief Attempt to gain exclusive access to the thread list.
 *
 *  @return Void.
 **/
inline void thrlist_lock(void)
{
  mutex_lock(&thread_list.lock);
  return;
}

/** @brief Relinquish exclusive access to the thread list.
 *
 *  @return Void.
 **/
inline void thrlist_unlock(void)
{
  mutex_unlock(&thread_list.lock);
  return;
}

