/* @file thread.c
 * 
 * @brief Defines structures and functions for thread manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug List locking mechanisms? 
 *      Should vm initialization be here?
 *      should thread init be responsible for acquiring the tid lock?
 *      extern thrlist and initialize here
 *
 */
#include <simics.h>

/* Pebble specific includes */
#include <cllist.h>
#include <pg_table.h>
#include <process.h>
#include <sched.h>
#include <sc_utils.h>
#include <thread.h>
#include <util.h>
#include <vm.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

/** @var thread_list 
 *  @brief The list of threads.
 **/
static cll_list thread_list = CLL_LIST_INITIALIZER(thread_list);
static mutex_s thrlist_lock = MUTEX_INITIALIZER(thrlist_lock);

/* For traversing the global thread list */
static cll_list *rover = NULL;

/* @brief Initialize a thread.
 *
 * Allocate a thread_t struct and atomically acquires a TID.
 *
 * @param task Task that thread belongs to.
 * @return Addres of initialized thread, or NULL if our of memory.
 */
thread_t *thread_init(task_t *task)
{
  assert(task);

  /* Allocate the thread structure */
  thread_t *thread = malloc(sizeof(thread_t));
  if(!thread) return NULL;

  /* Allocate the kernel stack */
  thread->kstack = smemalign(KSTACK_ALIGN, KSTACK_SIZE);
  if (!thread->kstack) {
    free(thread);
    return NULL;
  }

  /* Initialize other fields */
  mutex_init(&thread->lock);
  thread->state = THR_NASCENT;
  thread->task_info = task;
  thread->sp = NULL;
  thread->pc = NULL;
  thread->desched = THR_NOT_DESCHED;

  /* Embedded list traversal */
  cll_init_node(&thread->rq_entry, thread);
  cll_init_node(&thread->task_node, thread);
  cll_init_node(&thread->thrlist_entry, thread);

  /* No software exception handlers should be registered */
  swexn_deregister(&thread->swexn); 

  /* Add the thread to the thread list */
  assert(!thrlist_add(thread));
    
  return thread;
}

/** @brief Free a thread.
 *
 *  We free the thread's kernel stack, then the thread's TCB.
 *
 *  @param t The thread to free.
 *
 *  @return Void.
 **/
void thr_free(thread_t *t)
{
  sfree(t->kstack, PAGE_SIZE);
  free(t);
  return;
}

int thr_launch(thread_t *t, void *sp, void *pc)
{
  /* Set stack and instruction pointers */
  t->sp = sp;
  t->pc = pc;

  /* Unblock (also sets state to THR_RUNNABLE) */
  sched_unblock(t);

  return 0;
}


/*************************************************************************
 *  Thread List Functions - 1)
 *************************************************************************/

/** @brief Add a thread to the thread list and acquire a TID.
 *
 *  This function does an ordered insertion into the global thread list and
 *  assigns the thread the lowest available tid.  The rover points to the last
 *  inserted node.
 *
 *  @param t The thread to add.
 *
 *  @return -1 on error, else 0.
 **/
int thrlist_add(thread_t *t)
{
  cll_node *n;
  thread_t *curr;

  /* Avoid aggressive reuse of TIDs by keeping track of the 
   * last one assigned */
  static int tid = 0;

  /* Lock, insert, unlock */
  mutex_lock(&thrlist_lock);

  /* You are the only thread */
  if(cll_empty(&thread_list)){
    tid = 0;
    t->tid = ++tid;
    cll_insert(thread_list.next, &t->thrlist_entry);
    rover = (cll_list *)thread_list.next;
    mutex_unlock(&thrlist_lock);
    return 0;
  }

  /* Search for the lowest unused tid */
  cll_foreach(rover, n){
    /* We've reached the head of the list */
    if(n == &thread_list) break;
    curr = cll_entry(thread_t *, n);
    /* Gap found */
    if((curr->tid - tid) > 1) break;
    else tid = curr->tid;
  }

  /* We've rolled over, let's start recycling */
  if(tid == INT32_MAX){
    tid = 0;
    cll_foreach(&thread_list, n){
      /* We're back where we started */
      if(n == rover){
        mutex_unlock(&thrlist_lock);
        return -1;
      }
      curr = cll_entry(thread_t *, n);
      /* Gap found */
      if((curr->tid - tid) > 1) break;
      else tid = curr->tid;
    }
  }

  t->tid = ++tid;
  cll_insert(n, &t->thrlist_entry);

  /* Update the rover */
  rover = (cll_list *)n->prev;

  mutex_unlock(&thrlist_lock);

  return 0;
}

/** @brief Remove a thread from the thread list.
 *
 *  @param t The thread to delete.
 *
 *  @return -1 if thread not found, else 0.
 **/
int thrlist_del(thread_t *t)
{
  /* Lock, extract, unlock */
  mutex_lock(&thrlist_lock);

  /* Update the rover */
  if(&t->thrlist_entry == rover){
    rover = rover->prev;
  }
  assert(cll_extract(&thread_list, &t->thrlist_entry));

  mutex_unlock(&thrlist_lock);
  return 0;
}

/** @brief Search the thread list by TID for a specific thread.
 *
 *  @param tid The TID of the thread to look for.
 *
 *  @return A pointer to the thread, or NULL if not found.
 **/
thread_t *thrlist_find(int tid)
{
  cll_node *n;
  thread_t *t;

  t = NULL;

  /* Iteratively search the thread list */
  mutex_lock(&thrlist_lock);
  cll_foreach(&thread_list, n) {
    t = cll_entry(thread_t *,n);
    if (t->tid == tid) break;
  }
  mutex_unlock(&thrlist_lock);

  /* We didn't find it */
  if (n == &thread_list) return NULL;

  return t;
}

