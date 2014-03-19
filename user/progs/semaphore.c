/** @file semaphore.c
 *
 *  @brief The world's simplest semaphore test.
 *
 *  @author Enrique Naudon (esn)
 */

#include <sem.h>


int main(int argc, char *argv[])
{
  sem_t s;

  sem_init(&s,1);
  sem_wait(&s);
  sem_signal(&s);
  sem_destroy(&s);

	return 0;
}

