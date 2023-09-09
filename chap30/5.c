// 生产者/消费者（有界缓冲区）问题
// 以下代码存在问题
//当只有一个生产者和一个消费者时，代码能够正常运行。但如果有超过一个线程（例如两个消费者），这个方案会有两个严重的问题。
//假设有两个消费者（Tc1和Tc2），一个生产者（Tp）。首先，一个消费者（Tc1）先开始执行，
//它获得锁（c1），检查缓冲区是否可以消费（c2），然后等待（c3）（这会释放锁）。

//接着生产者（Tp）运行。它获取锁（p1），检查缓冲区是否满（p2），发现没满就给缓冲区加入一个数字（p4）。
//然后生产者发出信号，说缓冲区已满（p5）。关键的是，这让第一个消费者（Tc1）不再睡在条件变量上，进入就绪队列。
//Tc1现在可以运行（但还未运行）。生产者继续执行，直到发现缓冲区满后睡眠（p6,p1-p3）。

//问题发生了：另一个消费者（Tc2）抢先执行，消费了缓冲区中的值（c1,c2,c4,c5,c6，跳过了c3的等待，因为缓冲区是满的）。
//现在假设Tc1运行，在从wait返回之前，它获取了锁，然后返回。然后它调用了get() (p4)，但缓冲区已无法消费！断言触发

//问题产生的原因很简单：在Tc1被生产者唤醒后，但在它运行之前，缓冲区的状态改变了（由于Tc2）。
//发信号给线程只是唤醒它们，暗示状态发生了变化（在这个例子中，就是值已被放入缓冲区），但并不会保证在它运行之前状态一直是期望的情况。

#include "mythreads.h"

pthread_cond_t cond;
pthread_mutex_t mutex;
int loops = 1000000;
int count = 0;

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    Pthread_mutex_lock(&mutex);          // p1
    if (count == 1)                      // p2
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
    if (count == 0)                      // c2
      Pthread_cond_wait(&cond, &mutex);  // c3
    int tmp = get();                     // c4
    Pthread_cond_signal(&cond);          // c5
    Pthread_mutex_unlock(&mutex);        // c6
    printf("%d\n", tmp);
  }
}