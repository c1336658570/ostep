//链接的加载和条件式存储指令，实现锁

typedef struct lock_t {
  int flag;
} lock_t;

//C伪代码

//链接的加载
int LoadLinked(int *ptr) {
  return *ptr;
}

//条件存储，成功时，条件存储返回1，并将ptr指的值更新为value。失败时，返回0，并且不会更新值。
int StoreConditional(int *ptr, int value) {
  if (no one has updated *ptr since the LoadLinked to this address) {
    *ptr = value;
    return 1; // success!
  } else {
    return 0; // failed to update
  }
}

//一个线程调用lock()，执行了链接的加载指令，返回0。在执行条件式存储之前，中断产生了，另一个线程进入lock的代码，
//也执行链接式加载指令，同样返回0。现在，两个线程都执行了链接式加载指令，将要执行条件存储。
//重点是只有一个线程能够成功更新标志为1，从而获得锁；第二个执行条件存储的线程会失败（因为另一个线程已经成功执行了条件更新），
//必须重新尝试获取锁。
void lock(lock_t *lock) {
  while (1) {
    while (LoadLinked(&lock->flag) == 1)
      ; // spin until it's zero
    if (StoreConditional(&lock->flag, 1) == 1)
      return; // if set-it-to-1 was a success: all done
              // otherwise: try it all over again
  }
}

//等价于上面的lock
void lock(lock_t *lock) {
  while (LoadLinked(&lock->flag)||!StoreConditional(&lock->flag, 1))
    ; // spin
}

void unlock(lock_t *lock) {
  lock->flag = 0;
}