//测试并设置指令（原子交换）实现锁。
//存在两个问题：正确性和性能
//1.如果一个线程在如下第13行循环判断，然后退出循环，在执行第15行之前发生线程切换，另一个线程进入15行判断，退出循环
//这样就会造成多个线程持有锁，没有满足最基本的要求：互斥
//2.采用了自旋等待（spin-waiting）的技术，就是不停地检查标志的值。自旋等待在等待其他线程释放锁的时候会浪费时间。

typedef struct  lock_t { int flag; } lock_t;

void init(lock_t *mutex) {
// 0 -> lock is available, 1 -> held
  mutex->flag = 0;
}

void lock(lock_t *mutex) {
  while (mutex->flag == 1) // TEST the flag
    ; // spin-wait (do nothing)
  mutex->flag = 1;         // now SET it!
}

void unlock(lock_t *mutex) {
  mutex->flag = 0;
}