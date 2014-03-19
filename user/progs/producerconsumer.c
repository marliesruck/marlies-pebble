/** @file producerconsumer.c
 *
 *  @brief A simple producer-consumer test for semaphores.
 *
 *  This is the textbook solution for the producer-consumer problem using
 *  semaphores.  I did not come up with it myself, and use it only for
 *  testing.  The circular buffer implementation, however, is mine.
 *
 *  @author Enrique Naudon (esn)
 **/

#include <simics.h>

#include <mutex.h>
#include <sem.h>
#include <stdlib.h>
#include <syscall.h>
#include <thread.h>


#define BUFFER_SIZE 10
#define CHANCE 3

/** @brief A circular buffer.
 *
 *  Note: This was taken directly from my (Enrique's) P1.
 **/
struct circular_buffer {
  int r, w;                           /* Read/write indicies */
  int count;                          /* Num elements in buffer */
  unsigned int buffer[BUFFER_SIZE];   /* The buffer itself */
};
typedef struct circular_buffer cbuffer_s;

cbuffer_s buff = {
  .r = 0, .w = 0,
  .count = 0,
  .buffer = {0}
};

sem_t full, empty;
mutex_t mutex = MUTEX_INITIALIZER(mutex);

/** @brief Performs a wrapping increment.
 *
 *  Increments i, wrapping when i hits the buffer size.
 *
 *  @param i The value to increment
 *
 *  @return The incremented value
 **/
#define MODINC(i) ( ((i) + 1) % BUFFER_SIZE )

/** @brief Write a integer to the keyboard buffer.
 *
 *  Write the specified integer into the keyboard's buffer.  If there is
 *  no room in the buffer, overwrite the oldest integer.
 *
 *  @param n The integer to write into the buffer
 *
 *  @return Void.
 **/
static void buffer_write(unsigned int n)
{
  // Write into the buffer
  buff.buffer[buff.w] = n;
  buff.w = MODINC(buff.w);

  // Increment read as needed
  if (buff.count >= BUFFER_SIZE)
    buff.r = MODINC(buff.r);

  ++buff.count;
  return;
}

/** @brief Read a integer from the keyboard buffer.
 *
 *  We return the next integer in the buffer at the address specified by
 *  integer.  The return value indicates the number of lost (i.e.
 *  overwritten) entries since the last read.
 *
 *  @param n Destination pointer for read integer
 *
 *  @return The number of lost entries since the last read, or -1 if the
 *          buffer is currently empty.
 **/
static int buffer_read(unsigned int *n)
{
  int lost;
  
  /* Check for an empty buffer */
  if (buff.count == 0)
    return -1;

  /* Save lost and reset count */
  if (buff.count > BUFFER_SIZE) {
    lost = buff.count - BUFFER_SIZE;
    buff.count = BUFFER_SIZE;
  } else lost = 0;

  /* Read from the buffer */
  *n = buff.buffer[buff.r];
  buff.r = MODINC(buff.r);

  --buff.count;
  return lost;
}

/** @brief Produce numbers.
 **/
void *producer_fn(void *args)
{
  unsigned int i = 0;

  while (1)
  {
    ++i;

    sem_wait(&empty);
    mutex_lock(&mutex);

    buffer_write(i);

    mutex_unlock(&mutex);
    sem_signal(&full);

    lprintf("producer: %d was hard to make", i);
    if (!(rand() % CHANCE)) yield(-1);
  }

  return NULL;
}

/** @brief Consume numbers.
 **/
void *consumer_fn(void *args)
{
  unsigned int i = 0;
  unsigned int last = 0;
  int ret;

  while (1)
  {
    sem_wait(&full);
    mutex_lock(&mutex);

    ret = buffer_read(&i);

    mutex_unlock(&mutex);
    sem_signal(&empty);

    if (ret == -1) {
      lprintf("consumer: Hey!  I'm hungry!");
      return (void *)-1;
    }
    if (ret > 0) {
      lprintf("consumer: %d?  I'm missing %d!", i, ret);
      return (void *)-2;
    }
    else if (i != last + 1) {
      lprintf("consumer: %d?  I wanted %d!", i, last + 1);
      return (void *)-3;
    }
    else
      lprintf("consumer: %d was yummy", i);

    ++last;
    if (!(rand() % CHANCE)) yield(-1);
  }

  return NULL;
}

int main()
{
  int producer_tid, consumer_tid;
  int retval;

  thr_init(PAGE_SIZE);

  sem_init(&full, 0);
  sem_init(&empty, BUFFER_SIZE);

  consumer_tid = thr_create(consumer_fn, NULL);
  producer_tid = thr_create(producer_fn, NULL);

  thr_join(consumer_tid, (void **)&retval);

  lprintf("ERROR: The consumer returned!");
  if (retval == -1)
    lprintf("ERROR: We read an empty buffer!");
  if (retval == -2)
    lprintf("ERROR: We lost elements");
  if (retval == -3)
    lprintf("ERROR: We mangled elements");
  lprintf("ERROR: Aborting in failure");

  return -1;
}

