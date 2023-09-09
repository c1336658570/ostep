//5.c存在问题，使用While语句替代If

//这段代码仍然有一个问题

//假设两个消费者（Tc1和Tc2）先运行，都睡眠了（c3）。生产者开始运行，在缓冲区放入一个值，唤醒了一个消费者（假定是Tc1），
//并开始睡眠。现在是一个消费者马上要运行（Tc1），两个线程（Tc2和Tp）都等待在同一个条件变量上。

//因为消费者已经清空了缓冲区，很显然，应该唤醒生产者。但是，如果它唤醒了Tc2（这绝对是可能的，取决于等待队列是如何管理的），
//问题就出现了。具体来说，消费者Tc2会醒过来，发现队列为空（c2），又继续回去睡眠（c3）。生产者Tp刚才在缓冲区中放了一个值，
//现在在睡眠。另一个消费者线程Tc1也回去睡眠了。3个线程都在睡眠

#include "mythreads.h"

pthread_cond_t cond;
pthread_mutex_t mutex;
int loops = 1000000;
int count = 0;

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);          // p1
    while (count == 1)                      // p2
      Pthread_cond_wait(&cond, &mutex);  // p3
    put(i);                              // p4
    Pthread_cond_signal(&cond);          // p5
    Pthread_mutex_unlock(&mutex);        // p6
  }
}

void *consumer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);          // c1
    while (count == 0)                      // c2
      Pthread_cond_wait(&cond, &mutex);  // c3
    int tmp = get();                     // c4
    Pthread_cond_signal(&cond);          // c5
    Pthread_mutex_unlock(&mutex);        // c6
    printf("%d\n", tmp);
  }
}