#include "multifork.h"
#include <stdint.h>


/*  This is our equivalent to 
 *
 *
 */

uint64_t getsp( void )
{
    uint64_t sp;
    asm( "mov %%rsp, %0" : "=rm" ( sp ));
    return sp;
}

int mf_init(mf_struct *mf_data) {
  sem_init(&mf_data->sem, 0, 1);
  return 0;
}


void mf_block(mf_struct *mf_data) {
  char myobvious[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaa";
  pthread_t this_ptid = pthread_self();
  int found = 0;
  int index = 0;

  for (index = 0; index < mf_data->num_threads; index++) {
    if (mf_data->threads[index]  == this_ptid) {
      mf_data->blocked[index] = 1;
      found = 1;
      break;
    }
  }

  if (!found) {
    perror("TID not foundin mf_data!\n");
  }


  //sem_wait(&mf_data->sem);
  while(mf_data->blocked[index] == 1) {
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 2000;
    nanosleep(&req, NULL);
  }
}


pid_t multifork(mf_struct *mf_data) {
  assert(mf_data->num_threads <= MAX_THREADS);


  mf_memlayout mf_mem_layout;
  mf_mem_layout.num_threads = mf_data->num_threads;


  struct timespec req;

  for (int i = 0; i < mf_data->num_threads; i++) {
    // We must wait for thread to be blocked on us
    while(mf_data->blocked[i] == 0) {
      req.tv_sec = 0;
      req.tv_nsec = 2000;
      nanosleep(&req, NULL);
    }
    store_thread(mf_data->threads[i], &mf_mem_layout, i);
  }


  // Do our fork/clone whatever


  pid_t child_pid = fork();

  if (!child_pid) {
    // child
    sem_init(&mf_data->sem, 0, 1);
    for (int i = 0; i < mf_data->num_threads; i++) {
      restore_thread(i, &mf_mem_layout, mf_data);
    }
  }
  else {
    // cleanup 
    for (int i = 0; i < mf_mem_layout.num_threads; i++) {
      pthread_attr_destroy(&mf_mem_layout.attr[i]);
      mf_data->blocked[i] = 0;
    }
  }
  return child_pid;
} 

void store_thread(pthread_t t, mf_memlayout *mf_mem_layout, int i) {
  void *stackaddr;
  size_t stacksize; 

  if (pthread_getattr_np(t, &mf_mem_layout->attr[i]) != 0) {
    perror("Failed to call pthread_getattr\n");
    exit(2);
  }

  // Lets assume the thread stacks are unmodified, but unused
  pthread_attr_getstack(&mf_mem_layout->attr[i],&stackaddr, &stacksize);
  mf_mem_layout->stack_addr[i] = stackaddr;
  mf_mem_layout->stack_sz[i] = stacksize;
}

// Note: Overwrite mf_data
void restore_thread(int i, mf_memlayout *mf_mem_layout, mf_struct *mf_data) {
  void *ugly_ptr = mmap(NULL, sizeof(mf_memlayout) + sizeof(mf_struct) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  memcpy(ugly_ptr, (void *) &i, sizeof(int));
  memcpy(ugly_ptr + sizeof(int), (void *) mf_data, sizeof(mf_struct));
  memcpy(ugly_ptr + sizeof(int) + sizeof(mf_struct), (void *) mf_mem_layout, sizeof(mf_memlayout));



  void *stackaddr;
  size_t stacksize; 
  pthread_attr_getstack(&mf_mem_layout->attr[i],&stackaddr, &stacksize);
  mf_mem_layout->stack_addr[i] = stackaddr;
  mf_mem_layout->stack_sz[i] = stacksize;

  printf("saved stack: %lx\n", mf_mem_layout->stack_addr[i]);
  printf("saved stack sz: %ld\n", mf_mem_layout->stack_sz[i]);
  printf("attr stack: %lx\n", stackaddr);
  printf("attr stack sz: %ld\n", stacksize);

  mf_data->contexts[i].uc_stack.ss_sp = mf_mem_layout->stack_addr[i];
  mf_data->contexts[i].uc_stack.ss_size = mf_mem_layout->stack_sz[i];

  pthread_create(&mf_data->threads[i], &mf_mem_layout->attr[i], thread_entry, ugly_ptr);

  sleep(1);
  mf_data->blocked[i] = 0;
}

void *thread_entry(void *args) {
  // let's just pivot right over where we were
  __asm__(
      "push r10;"
      "mov r10, rsp;"
      "mov rbp, rsp;"
      "sub rbp, 0x18;"
      "sub r10, 0x80;"
      "mov r10, [r10];"
      "mov [rsp + 0x10], r10;"
      "pop r10;"
      "mov [rsp], rbp;"
  );

  return NULL;
}

