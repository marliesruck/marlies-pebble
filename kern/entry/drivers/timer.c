/** @file timer.c 
 *
 *  @brief A (very) basic timer driver.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No known bugs
 */

/* Timer includes */
#include <timer.h>

/* Pebble includes */
#include <interrupt_defines.h>
#include <sched.h>
#include <timer_defines.h>
#include <x86/asm.h>
#include <x86/seg.h>


volatile unsigned int ticks = 0;


#include <cllist.h>
#include <sched.h>

#include <assert.h>
#include <malloc.h>

static cll_list sleep_list = CLL_LIST_INITIALIZER(sleep_list);
static mutex_s sleep_lock = MUTEX_INITIALIZER(sleep_lock);

struct sleep_list_entry {
  thread_t *thread;
  unsigned int wake_time;
  cll_node *node;
};
typedef struct sleep_list_entry sl_entry;

int go_to_sleep(thread_t *t, unsigned int wake_time)
{
  cll_node n, *cursor;
  sl_entry ent, *temp;

  ent.thread = t;
  ent.wake_time = wake_time;
  ent.node = malloc(sizeof(cll_node));
  queue_init_node(ent.node, ent.thread);
  cll_init_node(&n, &ent);

  if (!ent.node) return -1;

  disable_interrupts();

  /* Find the spot we're inserting at */
  cll_foreach(&sleep_list, cursor) {
    temp = cll_entry(sl_entry *, cursor);
    if (temp->wake_time > wake_time) break;
  }

  cll_insert(cursor, &n);

  /* sched_block(...) will enable interrupts */
  return sched_block(t);
}

void wake_up(unsigned int time)
{
  sl_entry *sleeper;

  while (!cll_empty(&sleep_list))
  {
    sleeper = cll_entry(sl_entry *, sleep_list.next);
    if (sleeper->wake_time > time) break;
    assert(cll_extract(&sleep_list, sleep_list.next));

    raw_unblock(sleeper->thread, sleeper->node);
  }

  return;
}


/*************************************************************************/
/* External Interface                                                    */
/*************************************************************************/

/** @brief Handles timer interrupts.
 *
 *  @return Void.
 **/
void tmr_int_handler(void)
{
  /* Ack first, ask questions later
   * (Also increase tick count...)
   */
  outb(INT_CTL_PORT, INT_ACK_CURRENT);
  ticks += 1;

  wake_up(ticks);
  schedule();
  enable_interrupts();

  return;
}

/** @brief Initializes the timer driver.
 *
 *  @param rate The number of cycles per interrupt.
 *
 *  @return Void.
 **/
void tmr_init(unsigned short rate)
{
  outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
  outb(TIMER_PERIOD_IO_PORT, rate & 0xFF);
  outb(TIMER_PERIOD_IO_PORT, rate >> 8);

  return;
}

/** @brief Returns the number of ticks since boot.
 *
 *  It is important to note that the quantity/duration represented by a
 *  "tick" is entirely dependent on how the timer rate passed to
 *  tmr_init(...) when the timer driver was initialized.
 *
 *  @return The number of ticks since boot.
 **/
unsigned int tmr_get_ticks()
{
  unsigned int t;
  disable_interrupts();
  t = ticks;
  enable_interrupts();
  return t;
}

