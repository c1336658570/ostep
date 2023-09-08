//获取并增加
//最后一个硬件原语是获取并增加（fetch-and-add）指令，它能原子地返回特定地址的旧值，并且让该值自增一。获取并增加的C语言伪代码如下：

//如果线程希望获取锁，首先对一个ticket值执行一个原子的获取并相加指令。这个值作为该线程的“turn”（顺位，即myturn）。
//根据全局共享的lock->turn变量，当某一个线程的（myturn == turn）时，则轮到这个线程进入临界区。unlock则是增加turn，
//从而下一个等待线程可以进入临界区。

//不同于之前的方法：本方法能够保证所有线程都能抢到锁。只要一个线程获得了ticket值，它最终会被调度。之前的方法则不会保证。
//比如基于测试并设置的方法，一个线程有可能一直自旋，即使其他线程在获取和释放锁。

//用获取并增加指令，实现一个更有趣的ticket锁
int FetchAndAdd(int *ptr) {
  int old = *ptr;
  *ptr = old + 1;
  return old;
}
typedef struct  lock_t {
  int ticket;
  int turn;
} lock_t;

void lock_init(lock_t *lock) {
  lock->ticket = 0;
  lock->turn   = 0;
}

void lock(lock_t *lock) {
  int myturn = FetchAndAdd(&lock->ticket);
  while (lock->turn != myturn)
    ; // spin
}

void unlock(lock_t *lock) {
  FetchAndAdd(&lock->turn);
}