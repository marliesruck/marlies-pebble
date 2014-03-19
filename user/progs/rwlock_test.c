#include <rwlock.h>
#include <simics.h>
#include <thread.h>
#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_THREADS 1044
#define CHANCE 3
#define TOTAL_TIX 1000

/* Invariant: TOTAL_TIX - buyers = num_tix */
int num_tix = TOTAL_TIX;
int buyers = 0;

int tid[NUM_THREADS];

rwlock_t lock;

void *looking(void *param){
  rwlock_lock(&lock, RWLOCK_READ);
  lprintf("Customer %d is just looking",gettid());
  assert((TOTAL_TIX - buyers) == num_tix);
  rwlock_unlock(&lock);
  /* Force interleaving of instructions */
  if (!(rand() % CHANCE)) rwlock_unlock(&lock);
  return NULL;
}
void *sigdanger(void *param){
  rwlock_lock(&lock, RWLOCK_READ);
  lprintf("SIGDANGER SIGDANGER SIGDANGER\nDo you know how much memory you\n"
          "are using by spawning all these threads?!  Each thread gets its\n"
          "own stack and that's at least a page, you are so needy!");
  assert((TOTAL_TIX - buyers) == num_tix);
  rwlock_unlock(&lock);
  if (!(rand() % CHANCE)) rwlock_unlock(&lock);
  return NULL;
}
void *buyer(void *param){
  rwlock_lock(&lock, RWLOCK_WRITE);
  if(num_tix == 0)
    lprintf("All sold out :(");
  else{
    lprintf("tid: %d bought a ticket.  Now there are %d left",gettid(),--num_tix);
    buyers++;
  }
  assert((TOTAL_TIX - buyers) == num_tix);
  rwlock_unlock(&lock);
  if (!(rand() % CHANCE)) rwlock_unlock(&lock);
  return NULL;
}
void *indecisive(void *param){
  rwlock_lock(&lock, RWLOCK_WRITE);
  rwlock_downgrade(&lock);
  lprintf("These tixs are too expensive, I'm reneging!\n"
          "But now that I'm in the critical section, I'm just gonna hang out\n"
          "here and kill time because I don't want to study for my 410 exam.");
  assert((TOTAL_TIX - buyers) == num_tix);
  rwlock_unlock(&lock);
  if (!(rand() % CHANCE)) rwlock_unlock(&lock);
  return NULL;
}
int main(){
  thr_init(PAGE_SIZE);
  rwlock_init(&lock);

  int t;
  for(t = 0; t < NUM_THREADS; t+=6){
    tid[t] = thr_create(looking, NULL);
    if (!(rand() % CHANCE)) yield(-1);
    tid[t+1] = thr_create(buyer,NULL);
    if (!(rand() % CHANCE)) yield(-1);
    tid[t+2] = thr_create(looking, NULL);
    if (!(rand() % CHANCE)) yield(-1);
    tid[t+3] = thr_create(buyer,NULL);
    if (!(rand() % CHANCE)) yield(-1);
    tid[t+4] = thr_create(indecisive,NULL);
    if (!(rand() % CHANCE)) yield(-1);
    tid[t+5] = thr_create(sigdanger,NULL);
    if (!(rand() % CHANCE)) yield(-1);
  }

  for(t = 0; t < NUM_THREADS ; ++t)
    thr_join(tid[t],NULL);

  return 0;
}
