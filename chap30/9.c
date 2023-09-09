//覆盖条件
//当线程调用进入内存分配代码时，它可能会因为内存不足而等待。相应的，线程释放内存时，会发信号说有更多内存空闲。
//但是，代码中有一个问题：应该唤醒哪个等待线程（可能有多个线程）？

//假设目前没有空闲内存，线程Ta调用allocate(100)，接着线程Tb请求较少的内存，调用allocate(10)。
//Ta和Tb都等待在条件上并睡眠，没有足够的空闲内存来满足它们的请求。

//假定第三个线程Tc调用了free(50)。遗憾的是，当它发信号唤醒等待线程时，可能不会唤醒申请10字节的Tb线程。
//而Ta线程由于内存不够，仍然等待。因为不知道唤醒哪个（或哪些）线程，所以图中代码无法正常工作。

//用pthread_cond_broadcast()代替上述代码中的pthread_cond_signal()，唤醒所有的等待线程。
//这样做，确保了所有应该唤醒的线程都被唤醒。当然，不利的一面是可能会影响性能，因为不必要地唤醒了其他许多等待的线程，
//它们本来（还）不应该被唤醒。这些线程被唤醒后，重新检查条件，马上再次睡眠。

//这种条件变量叫作覆盖条件（covering condition），因为它能覆盖所有需要唤醒线程的场景（保守策略）。

#include "mythreads.h"
#define MAX_HEAP_SIZE 1024

// how many bytes of the heap are free?
int bytesLeft = MAX_HEAP_SIZE;

// need lock and condition too
pthread_cond_t c;
pthread_mutex_t m;

void *allocate(int size) {
  Pthread_mutex_lock(&m);
  while (bytesLeft < size) Pthread_cond_wait(&c, &m);
  void *ptr = ...;  // get mem from heap
  bytesLeft -= size;
  Pthread_mutex_unlock(&m);
  return ptr;
}

void free(void *ptr, int size) {
  Pthread_mutex_lock(&m);
  bytesLeft += size;
  Pthread_cond_signal(&c);  // whom to signal??
  Pthread_mutex_unlock(&m);
}