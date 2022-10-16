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
  sem_t sem;
} mf_struct;


typedef struct mf_memlayout {
  int num_threads;
  void *stack_addr[MAX_THREADS];
  size_t  stack_sz[MAX_THREADS];
  pthread_attr_t attr[MAX_THREADS];
} mf_memlayout;

pid_t multifork(mf_struct *mf_data);
void store_thread(pthread_t t, mf_memlayout *mf_mem_layout, int i);
void restore_thread(int i, mf_memlayout *mf_mem_layout, mf_struct *mf_data);
int mf_init(mf_struct *mf_data);
void mf_block(mf_struct *mf_data);


void *thread_entry(void *args);
