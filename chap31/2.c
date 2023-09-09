//二值信号量（锁）

#include <semaphore.h>

int main(void) {
  sem_t m;
  sem_init(&m, 0, 1);  // initialize semaphore to X; what should X be?

  sem_wait(&m);
  // critical section here
  sem_post(&m);
}