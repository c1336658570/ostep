//可扩展的计数

//懒惰计数器通过多个局部计数器和一个全局计数器来实现一个逻辑计数器，其中每个CPU核心有一个局部计数器。
//具体来说，在4个CPU的机器上，有4个局部计数器和1个全局计数器。除了这些计数器，还有锁：每个局部计数器有一个锁，全局计数器有一个。
//但是，为了保持全局计数器更新（以防某个线程要读取该值），局部值会定期转移给全局计数器，
//方法是获取全局锁，让全局计数器加上局部计数器的值，然后将局部计数器置零。
//这种局部转全局的频度，取决于一个阈值，这里称为S（表示sloppiness）。S越小，懒惰计数器则越趋近于非扩展的计数器。
//S越大，扩展性越强，但是全局计数器与实际计数的偏差越大。我们可以抢占所有的局部锁和全局锁（以特定的顺序，避免死锁），
//以获得精确值，但这种方法没有扩展性。
//阈值S为1024时懒惰计数器的性能。性能很高，4个处理器更新400万次的时间和一个处理器更新100万次的几乎一样。
//如果S小，性能很差（但是全局计数器精确度高）。如果S大，性能很好，但是全局计数器会有延时。懒惰计数器就是在准确性和性能之间折中。

#include <pthread.h>

#define NUMCPUS 8

typedef struct counter_t {
  int global;                      // global count
  pthread_mutex_t glock;           // global lock
  int local[NUMCPUS];              // local count (per cpu)
  pthread_mutex_t llock[NUMCPUS];  // ... and locks
  int threshold;                   // update frequency
} counter_t;

// init: record threshold, init locks, init values
//       of all local counts and global count
void init(counter_t *c, int threshold) {
  c->threshold = threshold;

  c->global = 0;
  pthread_mutex_init(&c->glock, NULL);

  int i;
  for (i = 0; i < NUMCPUS; i++) {
    c->local[i] = 0;
    pthread_mutex_init(&c->llock[i], NULL);
  }
}

// update: usually, just grab local lock and update local amount
//        once local count has risen by 'threshold', grab global
//        lock and transfer local values to it
void update(counter_t *c, int threadID, int amt) {
  pthread_mutex_lock(&c->llock[threadID]);
  c->local[threadID] += amt;                 // assumes amt > 0
  if (c->local[threadID] >= c->threshold) {  // transfer to global
    pthread_mutex_lock(&c->glock);
    c->global += c->local[threadID];
    pthread_mutex_unlock(&c->glock);
    c->local[threadID] = 0;
  }
  pthread_mutex_unlock(&c->llock[threadID]);
}

// get: just return global amount (which may not be perfect)
int get(counter_t *c) {
  pthread_mutex_lock(&c->glock);
  int val = c->global;
  pthread_mutex_unlock(&c->glock);
  return val;  // only approximate!
}