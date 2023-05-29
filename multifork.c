#include "multifork.h"
#include <stdint.h>
#include <stdint.h>
#include <sys/mman.h>

uint64_t getsp( void )
{
    uint64_t sp;
    asm( "mov %%rsp, %0" : "=rm" ( sp ));
    return sp;
}


mf_struct *mf_data;

mf_struct *mf_init() {
  mf_data = mmap(NULL, sizeof(mf_struct), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, 0,  0);
  mf_data->num_threads = 0;

  // TODO: Don't init unused semaphores
  for (int i = 0; i < MAX_THREADS; i++) {
    sem_init(&mf_data->sem[i], 1, 0);
  }
  return mf_data;
}


void* mf_thread_wrapper(void * args) {
  sem_wait( *(sem_t **) args);
  uint64_t* func = *(uint64_t**) (args + 8);
  ((void (*)(void*))(func))(NULL);
  pthread_exit(NULL);
}

int mfthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  sem_t sem;
  sem_init(&sem, 0, 0);

  // two pointers, this is dumb
  void **wrapper_arg = malloc(16);

  wrapper_arg[0] = &sem;
  wrapper_arg[1] = start_routine;

  mf_data->num_threads++;

  // TODO: target func arguments
  int ret = pthread_create(tid, attr, mf_thread_wrapper, wrapper_arg);
  mf_data->threads[0] = *tid;

  sem_post(&sem);
  return ret;
}


void mf_block() {
  pthread_t this_ptid = pthread_self();
  int found = 0;
  int index = 0;

  for (index = 0; index < mf_data->num_threads; index++) {
    if (mf_data->threads[index]  == this_ptid) {
      mf_data->blocked[index] = 1;
      found = 1;
      getcontext(&mf_data->contexts[index]);
      sem_wait(&mf_data->sem[index]);
      break;
    }
  }

  if (!found) {
    perror("TID not foundin mf_data!\n");
  }

}


pid_t multifork() {
  assert(mf_data->num_threads <= MAX_THREADS);

  for (int i = 0; i < mf_data->num_threads; i++) {
    // We must wait for thread to be blocked on us
    while(mf_data->blocked[i] == 0) {
      sleep(1);
    }
    store_thread(mf_data->threads[i], i);
  }

  pid_t child_pid = fork();

  if (!child_pid) {
    for (int i = 0; i < mf_data->num_threads; i++) {
      restore_thread(i);
    }
  }
  else {
    for (int i = 0; i < mf_data->num_threads; i++) {
      pthread_attr_destroy(&mf_data->attr[i]);
    }
  }
  return child_pid;
} 

void store_thread(pthread_t t, int i) {
  void *stackaddr;
  size_t stacksize; 

  void *newstackaddr;

  if (pthread_getattr_np(t, &mf_data->attr[i]) != 0) {
    perror("Failed to call pthread_getattr\n");
    exit(2);
  }

  // Get pthread stack and copy it before releasing
  pthread_attr_getstack(&mf_data->attr[i], &stackaddr, &stacksize);

  // Not mapped shared because it should be inherited via fork
  newstackaddr = mmap(NULL, stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_STACK, 0, 0);
  memcpy(newstackaddr, stackaddr, stacksize);

  mf_data->stack_addr[i] = newstackaddr;
  mf_data->stack_sz[i] = stacksize;
}

// Note: Overwrite mf_data
void restore_thread(int i) {
  void *stackaddr;
  size_t stacksize; 
  pthread_attr_getstack(&mf_data->attr[i], &stackaddr, &stacksize);
  mf_data->stack_addr[i] = stackaddr;
  mf_data->stack_sz[i] = stacksize;

#ifdef DEBUG
  printf("saved stack: %p\n", mf_data->stack_addr[i]);
  printf("saved stack sz: %ld\n", mf_data->stack_sz[i]);
  printf("attr stack: %p\n", stackaddr);
  printf("attr stack sz: %ld\n", stacksize);
#endif

  mf_data->contexts[i].uc_stack.ss_sp = mf_data->stack_addr[i];
  mf_data->contexts[i].uc_stack.ss_size = mf_data->stack_sz[i];

  pthread_create(&mf_data->threads[i], &mf_data->attr[i], thread_entry, (void *) &i);

  mf_data->blocked[i] = 0;
  sem_post(&mf_data->sem[i]);
}

void *thread_entry(void *args) {
  // TODO: fix the FUGLY 
  int index = *(int *)(args);
  sem_post(&mf_data->sem[index]);
  setcontext(&mf_data->contexts[index]);

  // setcontext does not return
  return NULL;
}

