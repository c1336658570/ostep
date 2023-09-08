//不同操作系统，不同实现
//Linux提供了futex，它类似于Solaris的接口，但提供了更多内核功能。具体来说，每个futex都关联一个特定的物理内存位置，
//也有一个事先建好的内核队列。调用者通过futex调用（见下面的描述）来睡眠或者唤醒。

//具体来说，有两个调用。调用futex_wait(address, expected)时，如果address处的值等于expected，
//就会让调用线程睡眠。否则，调用立刻返回。调用futex_wake(address)唤醒等待队列中的一个线程。


//利用一个整数，同时记录锁是否被持有（整数的最高位），以及等待者的个数（整数的其余所有位）。
//因此，如果锁是负的，它就被持有（因为最高位被设置，该位决定了整数的符号）。
//它展示了如何优化常见的情况，即没有竞争时：
//只有一个线程获取和释放锁，所做的工作很少（获取锁时测试和设置的原子位运算，释放锁时原子的加法）。

void mutex_lock (int *mutex) {
  int v;
  /* Bit 31 was clear, we got the mutex (this is the fastpath) */
  if (atomic_bit_test_set (mutex, 31) == 0)
    return;
  atomic_increment (mutex);
  while (1) {
    if (atomic_bit_test_set (mutex, 31) == 0) {
      atomic_decrement (mutex);
      return;
    }
    /* We have to wait now. First make sure the futex value
      we are monitoring is truly negative (i.e. locked). */
    v = *mutex;
    if (v >= 0)
      continue;
    futex_wait (mutex, v);
  }
}

void mutex_unlock (int *mutex) {
  /* Adding 0x80000000 to the counter results in 0 if and only if
    there are not other interested threads */
  if (atomic_add_zero (mutex, 0x80000000))
    return;

  /* There are other threads waiting for this mutex,
    wake one of them up. */
  futex_wake (mutex);
}

//两阶段锁
//Linux采用的是一种古老的锁方案，多年来不断被采用，可以追溯到20世纪60年代早期的Dahm锁[M82]，
//现在也称为两阶段锁（two-phase lock）。两阶段锁意识到自旋可能很有用，尤其是在很快就要释放锁的场景。
//因此，两阶段锁的第一阶段会先自旋一段时间，希望它可以获取锁。

//但是，如果第一个自旋阶段没有获得锁，第二阶段调用者会睡眠，直到锁可用。上文的Linux锁就是这种锁，不过只自旋一次；
//更常见的方式是在循环中自旋固定的次数，然后使用futex睡眠。

