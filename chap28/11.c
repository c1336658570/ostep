//使用队列：休眠替代自旋
//我们利用Solaris提供的支持，它提供了两个调用：park()能够让调用线程休眠，unpark(threadID)则会唤醒threadID标识的线程。
//可以用这两个调用来实现锁，让调用者在获取不到锁时睡眠，在锁可用时被唤醒。

//将之前的测试并设置和等待队列结合，实现了一个更高性能的锁。其次，我们通过队列来控制谁会获得锁，避免饿死。
//guard基本上起到了自旋锁的作用，围绕着flag和队列操作。因此，这个方法并没有完全避免自旋等待。
//线程在获取锁或者释放锁时可能被中断，从而导致其他线程自旋等待。但是，这个自旋等待时间是很有限的
//（不是用户定义的临界区，只是在lock和unlock代码中的几个指令），因此，这种方法也许是合理的。

//在lock()函数中，如果线程不能获取锁（它已被持有），线程会把自己加入队列（通过调用gettid()获得当前的线程ID），
//将guard设置为0，然后让出CPU。留给读者一个问题：如果我们在park()之后，才把guard设置为0释放锁，会发生什么呢？
//死锁现象

//当要唤醒另一个线程时，flag并没有设置为0。为什么呢？其实这不是错，而是必须的！线程被唤醒时，就像是从park()调用返回。
//但是，此时它没有持有guard，所以也不能将flag设置为1。因此，我们就直接把锁从释放的线程传递给下一个获得锁的线程，期间flag不必设置为0。

//在park()调用之前。如果不凑巧，一个线程将要park，假定它应该睡到锁可用时。这时切换到另一个线程（比如持有锁的线程），
//这可能会导致麻烦。比如，如果该线程随后释放了锁。接下来第一个线程的park会永远睡下去（可能）。
//这种问题有时称为唤醒/等待竞争（wakeup/waiting race）。为了避免这种情况，我们需要额外的工作。

//Solaris通过增加了第三个系统调用separk()来解决这一问题。通过setpark()，一个线程表明自己马上要park。
//如果刚好另一个线程被调度，并且调用了unpark，那么后续的park调用就会直接返回，而不是一直睡眠。lock()调用可以做一点小修改：
queue_add(m->q, gettid());
setpark(); // new code
m->guard = 0;

typedef struct  lock_t {
  int flag;
  int guard;
  queue_t *q;
} lock_t;

void lock_init(lock_t *m) {
  m->flag = 0;
  m->guard = 0;
  queue_init(m->q);
}

void lock(lock_t *m) {
  while (TestAndSet(&m->guard, 1) == 1)
    ; //acquire guard lock by spinning
  if (m->flag == 0) {
    m->flag = 1; // lock is acquired
    m->guard = 0;
  } else {
    queue_add(m->q, gettid());
    m->guard = 0;
    park();
  }
}

void unlock(lock_t *m) {
  while (TestAndSet(&m->guard, 1) == 1)
    ; //acquire guard lock by spinning
  if (queue_empty(m->q))
    m->flag = 0; // let go of lock; no one wants it
  else
    unpark(queue_remove(m->q)); // hold lock (for next thread!)
  m->guard = 0;
}