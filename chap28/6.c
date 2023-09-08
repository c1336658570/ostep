//测试并设置指令（test-and-set）实现一个自旋锁。实现了一个有效的互斥原语！
//这是最简单的一种锁，一直自旋，利用CPU周期，直到锁可用。在单处理器上，需要抢占式的调度器
//（preemptive scheduler，即不断通过时钟中断一个线程，运行其他线程）。
//否则，自旋锁在单CPU上无法使用，因为一个自旋的线程永远不会放弃CPU。

//在SPARC上，测试并设置指令（test-and-set）叫ldstub（load/store unsigned byte，加载/保存无符号字节）
//在x86上，是xchg（atomic exchange，原子交换）指令

//它返回old_ptr指向的旧值，同时更新为new的新值。当然，关键是这些代码是原子地（atomically）执行。
//因为既可以测试旧值，又可以设置新值，所以我们把这条指令叫作“测试并设置”。这一条指令完全可以实现一个简单的自旋锁（spin lock）
int TestAndSet(int *old_ptr, int new) {
  int old = *old_ptr; // fetch old value at old_ptr
  *old_ptr = new;    // store 'new' into old_ptr
  return old;        // return the old value
}

typedef struct  lock_t {
  int flag;
} lock_t;

void init(lock_t *lock) {
  // 0 indicates that lock is available, 1 that it is held
  lock->flag = 0;
}
void lock(lock_t *lock) {
  while (TestAndSet(&lock->flag, 1) == 1)
    ; // spin-wait (do nothing)
}

void unlock(lock_t *lock) {
  lock->flag = 0;
}

//评价自旋锁
//1/锁最重要的一点是正确性（correctness）：能够互斥吗？答案是可以的：自旋锁一次只允许一个线程进入临界区。因此，这是正确的锁。
//2.下一个标准是公平性（fairness）。自旋锁对于等待线程的公平性如何呢？能够保证一个等待线程会进入临界区吗？
//答案是自旋锁不提供任何公平性保证。实际上，自旋的线程在竞争条件下可能会永远自旋。自旋锁没有公平性，可能会导致饿死。
//3.最后一个标准是性能（performance）。使用自旋锁的成本是多少？为了更小心地分析，我们建议考虑几种不同的情况。
//首先，考虑线程在单处理器上竞争锁的情况。然后，考虑这些线程跨多个处理器。

//对于自旋锁，在单CPU的情况下，性能开销相当大。假设一个线程持有锁进入临界区时被抢占。
//调度器可能会运行其他每一个线程（假设有N−1个这种线程）。而其他线程都在竞争锁，
//都会在放弃CPU之前，自旋一个时间片，浪费CPU周期。
//在多CPU上，自旋锁性能不错（如果线程数大致等于CPU数）。假设线程A在CPU 1，线程B在CPU 2竞争同一个锁。线程A（CPU 1）占有锁时，
//线程B竞争锁就会自旋（在CPU 2上）。然而，临界区一般都很短，因此很快锁就可用，然后线程B获得锁。自旋等待其他处理器上的锁，
//并没有浪费很多CPU周期，因此效果不错。