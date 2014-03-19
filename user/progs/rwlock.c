#include <rwlock.h>
#include <simics.h>
#include <thread.h>
#include <syscall.h>
#include <stdio.h>
#include <string.h>

#define NUM_ELEM 4
#define BUF_SIZE 256
#define NUM_THREADS 8

char buf[BUF_SIZE];
unsigned int offset = 0;

char *array[] = {"summer","spring","frolic","play"};
int tid[NUM_THREADS];

rwlock_t lock;

void *reader(void *param){
  while(buf == NULL)
    yield(-1);
  lprintf("reader locking...");
  rwlock_lock(&lock, RWLOCK_READ);
  lprintf("reader %d reads: %s",gettid(),buf);
  rwlock_unlock(&lock);
  lprintf("read released lock");
  return NULL;
}
void *writer(void *param){
  int i = (int)(param);
  lprintf("writer locking...");
  rwlock_lock(&lock, RWLOCK_WRITE);
  lprintf("writer locked");
  char *pos = (char *)((unsigned)(buf) + offset);
  sprintf(pos,"%s",array[i]);
  offset+= strlen(array[i]);
  lprintf("writer unlocking...");
  rwlock_unlock(&lock);
  lprintf("writer unlocked");
  return NULL;
}
int main(){
  thr_init(PAGE_SIZE);
  rwlock_init(&lock);

  int t = 0;
  /* Spawn readers */
  //for(t = 0; t < NUM_ELEM; ++t){
  //}
  //for(t = 4; t < NUM_THREADS; ++t){
    tid[t] = thr_create(writer, (void *)(0));
    lprintf("spawned writer");
    t++;
    tid[t] = thr_create(reader,NULL);
    lprintf("spawned reader");
    //yield(tid[t-NUM_ELEM]);
  //}
  for(t = 0; t < 2 ; ++t)
    thr_join(tid[t],NULL);
  return 0;
}
