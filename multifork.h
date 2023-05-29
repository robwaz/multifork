#define _GNU_SOURCE

#include  <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MAX_THREADS 16

typedef struct mf_struct {
  int num_threads;
  pthread_t threads[MAX_THREADS];
  int blocked[MAX_THREADS];
  ucontext_t contexts[MAX_THREADS];
  sem_t sem[MAX_THREADS];
  void *stack_addr[MAX_THREADS];
  size_t  stack_sz[MAX_THREADS];
  pthread_attr_t attr[MAX_THREADS];
} mf_struct;

pid_t multifork();
void store_thread(pthread_t t, int i);
void restore_thread(int i);
mf_struct *mf_init();
void mf_block();
int mfthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
void* mf_thread_wrapper(void *);


void *thread_entry(void *args);
