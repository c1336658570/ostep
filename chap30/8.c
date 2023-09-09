//最终的生产者/消费者方案
//增加更多缓冲区槽位，这样在睡眠之前，可以生产多个值。同样，睡眠之前可以消费多个值。
//单个生产者和消费者时，这种方案因为上下文切换少，提高了效率。
//多个生产者和消费者时，它甚至支持并发生产和消费，从而提高了并发。

#include "mythreads.h"

#define MAX 10

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;

void put(int value) {
  buffer[fill_ptr] = value;
  fill_ptr = (fill_ptr + 1) % MAX;
  count++;
}

int get() {
  int tmp = buffer[use_ptr];
  use_ptr = (use_ptr + 1) % MAX;
  count--;
  return tmp;
}

pthread_cond_t empty, fill;
pthread_mutex_t mutex;
int loops = 1000000;

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);           // p1
    while (count == MAX)                  // p2
      Pthread_cond_wait(&empty, &mutex);  // p3
    put(i);                               // p4
    Pthread_cond_signal(&fill);           // p5
    Pthread_mutex_unlock(&mutex);         // p6
  }
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);          // c1
    while (count == 0)                   // c2
      Pthread_cond_wait(&fill, &mutex);  // c3
    int tmp = get();                     // c4
    Pthread_cond_signal(&empty);         // c5
    Pthread_mutex_unlock(&mutex);        // c6
    printf("%d\n", tmp);
  }
}