//解决4.c的问题，增加互斥
//给整个put()/get()部分都增加了锁，注释中有NEW LINE的几行就是。这似乎是正确的思路，但仍然有问题。

//会发生死锁

//假设有两个线程，一个生产者和一个消费者。消费者首先运行，获得锁（c0行），然后对full信号量执行sem_wait()（c1行）。
//因为还没有数据，所以消费者阻塞，让出CPU。但是，重要的是，此时消费者仍然持有锁。

//然后生产者运行。假如生产者能够运行，它就能生产数据并唤醒消费者线程。遗憾的是，它首先对二值互斥信号量调用sem_wait()（p0行）。
//锁已经被持有，因此生产者也被卡住。

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
    sem_wait(&mutex);  // line p0 (NEW LINE)
    sem_wait(&empty);  // line p1
    put(i);            // line p2
    sem_post(&full);   // line p3
    sem_post(&mutex);  // line p4 (NEW LINE)
  }
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    sem_wait(&mutex);  // line c0 (NEW LINE)
    sem_wait(&full);   // line c1
    int tmp = get();   // line c2
    sem_post(&empty);  // line c3
    sem_post(&mutex);  // line c4 (NEW LINE)
    printf("%d\n", tmp);
  }
}

int main(int argc, char *argv[]) {
  // ...
  sem_init(&empty, 0, MAX);  // MAX buffers are empty to begin with...
  sem_init(&full, 0, 0);     // ... and 0 are full
  sem_init(&mutex, 0, 1);    // mutex=1 because it is a lock (NEW LINE)
                             // ...
}