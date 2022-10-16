#include <pthread.h>
#include "multifork.h"
#include <unistd.h>


mf_struct mf_data;

void *sleep_loop(void *nope) {

  sleep(3);
  mf_block(&mf_data);

  printf("pthread %ld ending\n", pthread_self());

  return NULL;
}

int main() {
  pthread_t tid;

  mf_init(&mf_data);
  pthread_create(&tid, NULL, sleep_loop, NULL);
  mf_data.num_threads = 1;
  mf_data.threads[0] = tid;

  pid_t child_pid = multifork(&mf_data);

  if (!child_pid) {
    // child 

    printf("example: %d is about to sleep\n", getpid());
    sleep(60);
    printf("example: %d returns from sleep\n", getpid());
  }
  else {
    waitpid(child_pid, NULL, 0);
    printf("example: child_pid %d done\n", child_pid);
  }
}






