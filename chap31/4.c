//生产者/消费者（有界缓冲区）问题

#include <semaphore.h>

// 第一次尝试



//假设MAX=1（数组中只有一个缓冲区），验证程序是否有效。

//假设有两个线程，一个生产者和一个消费者。我们来看在一个CPU上的具体场景。消费者先运行，执行到C1行，
//调用sem_wait(&full)。因为full初始值为0，wait调用会将full减为−1，导致消费者睡眠，
//等待另一个线程调用sem_post(&full)，符合预期。

//假设生产者然后运行。执行到P1行，调用sem_wait(&empty)。不像消费者，生产者将继续执行，
//因为empty被初始化为MAX（在这里是1）。因此，empty被减为0，生产者向缓冲区中加入数据，然后执行P3行，
//调用sem_post(&full)，把full从−1变成0，唤醒消费者（即将它从阻塞变成就绪）。

//在这种情况下，可能会有两种情况。如果生产者继续执行，再次循环到P1行，由于empty值为0，它会阻塞。如果生产者被中断，
//而消费者开始执行，调用sem_wait(&full)（c1行），发现缓冲区确实满了，消费它。这两种情况都是符合预期的。

//可以用更多的线程来尝试这个例子（即多个生产者和多个消费者）。它应该仍然正常运行。




//假设MAX大于1（比如MAX=10）。对于这个例子，假定有多个生产者，多个消费者。现在就有问题了：竞态条件。
//假设两个生产者（Pa和Pb）几乎同时调用put()。当Pa先运行，在f1行先加入第一条数据（fill=0），
//假设Pa在将fill计数器更新为1之前被中断，Pb开始运行，也在f1行给缓冲区的0位置加入一条数据，这意味着那里的老数据被覆盖！


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

void *producer(void *arg) {
  int i;
  for (i = 0; i < loops; i++) {
    sem_wait(&empty);  // line P1
    put(i);            // line P2
    sem_post(&full);   // line P3
  }
}

void *consumer(void *arg) {
  int i, tmp = 0;
  while (tmp != -1) {
    sem_wait(&full);   // line C1
    tmp = get();       // line C2
    sem_post(&empty);  // line C3
    printf("%d\n", tmp);
  }
}

int main(int argc, char *argv[]) {
  // ...
  sem_init(&empty, 0, MAX);  // MAX buffers are empty to begin with...
  sem_init(&full, 0, 0);     // ... and 0 are full
                             // ...
}