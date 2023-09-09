//解决5.c的问题，避免死锁

#include <semaphore.h>

#define MAX 10
int loops = 100000;

int buffer[MAX];
int fill = 0;
int use = 0;

void put(int value) {
  buffer[fill] = value;     // line f1
  fill = (fill + 1) % MAX;  // line f2
}

int get() {
  int tmp = buffer[use];  // line g1
  use = (use + 1) % MAX;  // line g2
  return tmp;
}

sem_t empty;
sem_t full;
sem_t mutex;

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    sem_wait(&empty);  // line p1
    sem_wait(&mutex);  // line p1.5 (MOVED MUTEX HERE...)
    put(i);            // line p2
    sem_post(&mutex);  // line p2.5 (... AND HERE)
    sem_post(&full);   // line p3
  }
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    sem_wait(&full);   // line c1
    sem_wait(&mutex);  // line c1.5 (MOVED MUTEX HERE...)
    int tmp = get();   // line c2
    sem_post(&mutex);  // line c2.5 (... AND HERE)
    sem_post(&empty);  // line c3
    printf("%d\n", tmp);
  }
}

int main(int argc, char *argv[]) {
  // ...
  sem_init(&empty, 0, MAX);  // MAX buffers are empty to begin with...
  sem_init(&full, 0, 0);     // ... and 0 are full
  sem_init(&mutex, 0, 1);    // mutex=1 because it is a lock
                             // ...
}