#include <pthread.h>
#include "multifork.h"
#include <unistd.h>


void *sleep_loop(void *nope) {

  mf_block();
  while (1) {
    sleep(3);

    printf("pthread %0lx in pid %d looping\n", pthread_self(), getpid());
  }

  return NULL;
}

int main() {
  pthread_t tid;
  mf_init();

  mfthread_create(&tid, NULL, sleep_loop, NULL);

  pid_t child_pid = multifork();


  if (!child_pid) {
    pthread_join(tid, NULL);
  }
  else {
    printf("Parent PID: %d\n", getpid());
    printf("Child PID: %d\n", child_pid);
    waitpid(child_pid, NULL, 0);
    pthread_join(tid, NULL);
    printf("example: child_pid %d done\n", child_pid);
  }
}
