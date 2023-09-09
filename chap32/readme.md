# 常见并发问题

## 有哪些类型的缺陷

非死锁的缺陷和死锁缺陷

## 非死锁缺陷

违反原子性（atomicity violation）缺陷和错误顺序（order violation）缺陷。

1. 违反原子性缺陷

```c
Thread 1::
  if (thd->proc_info) {
    ...
    fputs(thd->proc_info, ...);
    ...
  }

  Thread 2::
  thd->proc_info = NULL;
```

两个线程都要访问thd结构中的成员proc_info。第一个线程检查proc_info非空，然后打印出值；第二个线程设置其为空。显然，当第一个线程检查之后，在fputs()调用之前被中断，第二个线程把指针置为空；当第一个线程恢复执行时，由于引用空指针，导致程序奔溃。

“违反了多次内存访问中预期的可串行性（即代码段本意是原子的，但在执行中并没有强制实现原子性）”。在我们的例子中，proc_info的非空检查和fputs()调用打印proc_info是假设原子的，当假设不成立时，代码就出问题了。

只要给共享变量的访问加锁，确保每个线程访问proc_info字段时，都持有锁（proc_info_lock）

```c
pthread_mutex_t proc_info_lock = PTHREAD_MUTEX_INITIALIZER;

Thread 1 ::pthread_mutex_lock(&proc_info_lock);
if (thd->proc_info) {
  ... fputs(thd->proc_info, ...);
  ...
}
pthread_mutex_unlock(&proc_info_lock);

Thread 2 ::pthread_mutex_lock(&proc_info_lock);
thd->proc_info = NULL;
pthread_mutex_unlock(&proc_info_lock);
```

2. 违反顺序缺陷

```c
Thread 1 ::void init() {
  ... 
  mThread = PR_CreateThread(mMain, ...);
  ...
}

Thread 2 ::void mMain(...) {
  ... 
  mState = mThread->State;
  ...
}
```

线程2的代码中似乎假定变量mThread已经被初始化了（不为空）。然而，如果线程1并没有首先执行，线程2就可能因为引用空指针奔溃（假设mThread初始值为空；否则，可能会产生更加奇怪的问题，因为线程2中会读到任意的内存位置并引用）。

“两个内存访问的预期顺序被打破了（即A应该在B之前执行，但是实际运行中却不是这个顺序）”

通过强制顺序来修复这种缺陷。条件变量（condition variables）就是一种简单可靠的方式

```c
pthread_mutex_t mtLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mtCond = PTHREAD_COND_INITIALIZER;
int mtInit = 0;

Thread 1 ::void init() {
  ... 
  mThread = PR_CreateThread(mMain, ...);

  // signal that the thread has been created...
  pthread_mutex_lock(&mtLock);
  mtInit = 1;
  pthread_cond_signal(&mtCond);
  pthread_mutex_unlock(&mtLock);
  ...
}

Thread 2 ::void mMain(...) {
  ...
  // wait for the thread to be initialized...
  pthread_mutex_lock(&mtLock);
  while (mtInit == 0) pthread_cond_wait(&mtCond, &mtLock);
  pthread_mutex_unlock(&mtLock);

  mState = mThread->State;
  ...
}
```

## 非死锁缺陷：小结

大部分（97%）的非死锁问题是违反原子性和违反顺序这两种。

## 死锁缺陷

当线程1持有锁L1，正在等待另外一个锁L2，而线程2持有锁L2，却在等待锁L1释放时，死锁就产生了。

```c
Thread 1:    Thread 2:
lock(L1);    lock(L2);
lock(L2);    lock(L1);
```

### 为什么发生死锁

其中一个原因是在大型的代码库里，组件之间会有复杂的依赖。以操作系统为例。虚拟内存系统在需要访问文件系统才能从磁盘读到内存页；文件系统随后又要和虚拟内存交互，去申请一页内存，以便存放读到的块。因此，在设计大型系统的锁机制时，你必须要仔细地去避免循环依赖导致的死锁。

另一个原因是封装（encapsulation）。软件开发者一直倾向于隐藏实现细节，以模块化的方式让软件开发更容易。然而，模块化和锁不是很契合。Jula等人指出[J+08]，某些看起来没有关系的接口可能会导致死锁。以Java的Vector类和AddAll()方法为例，我们这样调用这个方法：

```java
Vector v1, v2; 
v1.AddAll(v2);
```

在内部，这个方法需要多线程安全，因此针对被添加向量（v1）和参数（v2）的锁都需要获取。假设这个方法，先给v1加锁，然后再给v2加锁。如果另外某个线程几乎同时在调用v2.AddAll(v1)，就可能遇到死锁。

### 产生死锁的条件

- 互斥：线程对于需要的资源进行互斥的访问（例如一个线程抢到锁）。
- 持有并等待：线程持有了资源（例如已将持有的锁），同时又在等待其他资源（例如，需要获得的锁）。
- 非抢占：线程获得的资源（例如锁），不能被抢占。
- 循环等待：线程之间存在一个环路，环路上每个线程都额外持有一个资源，而这个资源又是下一个线程要申请的。

### 预防

#### 循环等待

就是让代码不会产生循环等待。最直接的方法就是获取锁时提供一个全序（total ordering）。假如系统共有两个锁（L1和L2），那么我们每次都先申请L1然后申请L2，就可以避免死锁。这样严格的顺序避免了循环等待，也就不会产生死锁。

更复杂的系统中不会只有两个锁，锁的全序可能很难做到。因此，偏序（partial ordering）可能是一种有用的方法，安排锁的获取并避免死锁。Linux中的内存映射代码就是一个偏序锁的好例子[T+94]。代码开头的注释表明了10组不同的加锁顺序，包括简单的关系，比如i_mutex早于i_mmap_mutex，也包括复杂的关系，比如i_mmap_mutex早于private_lock，早于swap_lock，早于mapping->tree_lock。

你可以想到，全序和偏序都需要细致的锁策略的设计和实现。另外，顺序只是一种约定，粗心的程序员很容易忽略，导致死锁。最后，有序加锁需要深入理解代码库，了解各种函数的调用关系，即使一个错误，也会导致“Deadlock”字。

##### 通过锁的地址来强制锁的顺序

当一个函数要抢多个锁时，我们需要注意死锁。比如有一个函数：do_something(mutex t *m1, mutex t *m2)，如果函数总是先抢m1，然后m2，那么当一个线程调用do_something(L1, L2)，而另一个线程调用do_something(L2, L1)时，就可能会产生死锁。

为了避免这种特殊问题，聪明的程序员根据锁的地址作为获取锁的顺序。按照地址从高到低，或者从低到高的顺序加锁，do_something()函数就可以保证不论传入参数是什么顺序，函数都会用固定的顺序加锁。具体的代码如下：

```c
if (m1 > m2) { // grab locks in high-to-low address order 
  pthread_mutex_lock(m1);
  pthread_mutex_lock(m2);
} else { 
  pthread_mutex_lock(m2); 
  pthread_mutex_lock(m1);
　
}
// Code assumes that m1 != m2 (it is not the same lock)
```

#### 持有并等待

可以通过原子地抢锁来避免。

```c
lock(prevention);
lock(L1);
lock(L2);
... 
unlock(prevention);
```

先抢到prevention这个锁之后，代码保证了在抢锁的过程中，不会有不合时宜的线程切换，从而避免了死锁。当然，这需要任何线程在任何时候抢占锁时，先抢到全局的prevention锁。例如，如果另一个线程用不同的顺序抢锁L1和L2，也不会有问题，因为此时，线程已经抢到了prevention锁。

出于某些原因，这个方案也有问题。和之前一样，它不适用于封装：因为这个方案需要我们准确地知道要抢哪些锁，并且提前抢到这些锁。因为要提前抢到所有锁（同时），而不是在真正需要的时候，所以可能降低了并发。

#### 非抢占

在调用unlock之前，都认为锁是被占有的，多个抢锁操作通常会带来麻烦，因为我们等待一个锁时，同时持有另一个锁。很多线程库提供更为灵活的接口来避免这种情况。具体来说，trylock()函数会尝试获得锁，或者返回−1，表示锁已经被占有。你可以稍后重试一下。

```c
top : 
lock(L1);
if (trylock(L2) == -1) {
  unlock(L1);
  goto top;
}
```

另一个线程可以使用相同的加锁方式，但是不同的加锁顺序（L2然后L1），程序仍然不会产生死锁。但是会引来一个新的问题：活锁（livelock）。两个线程有可能一直重复这一序列，又同时都抢锁失败。这种情况下，系统一直在运行这段代码（因此不是死锁），但是又不会有进展，因此名为活锁。也有活锁的解决方法：例如，可以在循环结束的时候，先随机等待一个时间，然后再重复整个动作，这样可以降低线程之间的重复互相干扰。

使用trylock方法可能会有一些困难。第一个问题仍然是封装：如果其中的某一个锁，是封装在函数内部的，那么这个跳回开始处就很难实现。如果代码在中途获取了某些资源，必须要确保也能释放这些资源。例如，在抢到L1后，我们的代码分配了一些内存，当抢L2失败时，并且在返回开头之前，需要释放这些内存。当然，在某些场景下（例如，之前提到的Java的vector方法），这种方法很有效。

#### 互斥

设计各种无等待（wait-free）数据结构的思想。想法很简单：通过强大的硬件指令，我们可以构造出不需要锁的数据结构。

假设我们有比较并交换（compare-and-swap）指令，是一种由硬件提供的原子指令，做下面的事：

```c
int CompareAndSwap(int *address, int expected, int new) {
  if (*address == expected) {
    *address = new;
    return 1;  // success
  }
  return 0;  // failure
}
```

假定我们想原子地给某个值增加特定的数量。我们可以这样实现：

```c
void AtomicIncrement(int *value, int amount) {
  do {
    int old = *value;
  } while (CompareAndSwap(value, old, old + amount) == 0);
}
```

无须获取锁，更新值，然后释放锁这些操作，我们使用比较并交换指令，反复尝试将值更新到新的值。这种方式没有使用锁，因此不会有死锁（有可能产生活锁）。

考虑一个更复杂的例子：链表插入。

```c
void insert(int value) {
  node_t *n = malloc(sizeof(node_t));
  assert(n != NULL);
  n->value = value;
  n->next = head;
  head = n;
}
```

这段代码在多线程同时调用的时候，会有临界区（看看你是否能弄清楚原因）。当然，我们可以通过给相关代码加锁，来解决这个问题：

```c
void insert(int value) {
  node_t *n = malloc(sizeof(node_t));
  assert(n != NULL);
  n->value = value;
  lock(listlock);  // begin critical section
  n->next = head;
  head = n;
  unlock(listlock);  // end of critical section
}
```

上面的方案中，我们使用了传统的锁。这里我们尝试用比较并交换指令（compare-and-swap)来实现插入操作。一种可能的实现是：

```c
void insert(int value) {
  node_t *n = malloc(sizeof(node_t));
  assert(n != NULL);
  n->value = value;
  do {
    n->next = head;
  } while (CompareAndSwap(&head, n->next, n) == 0);
}
```

这段代码，首先把next指针指向当前的链表头（head），然后试着把新节点交换到链表头。但是，如果此时其他的线程成功地修改了head的值，这里的交换就会失败，导致这个线程根据新的head值重试。

### 通过调度避免死锁

假设我们需要在两个处理器上调度4个线程。更进一步，假设我们知道线程1（T1）需要用锁L1和L2，T2也需要抢L1和L2，T3只需要L2，T4不需要锁。

一种比较聪明的调度方式是，只要T1和T2不同时运行，就不会产生死锁。下面就是这种方式：

CPU1：T3	T4

CPU2：T1	T2

T3和T1重叠，或者和T2重叠都是可以的。虽然T3会抢占锁L2，但是由于它只用到一把锁，和其他线程并发执行都不会产生死锁。

### 检查和恢复

允许死锁偶尔发生，检查到死锁时再采取行动。举个例子，如果一个操作系统一年死机一次，你会重启系统，然后愉快地（或者生气地）继续工作。如果死锁很少见，这种不是办法的办法也是很实用的。

很多数据库系统使用了死锁检测和恢复技术。死锁检测器会定期运行，通过构建资源图来检查循环。当循环（死锁）发生时，系统需要重启。如果还需要更复杂的数据结构相关的修复，那么需要人工参与。
