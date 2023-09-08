//比较并交换，实现自旋锁
//某些系统提供了另一个硬件原语，即比较并交换指令（SPARC系统中是compare-and-swap，x86系统是compare-and-exchange）。
//如下是这条指令的C语言伪代码。

typedef struct lock_t {
  int flag;
} lock_t;

//检测ptr指向的值是否和expected相等；如果是，更新ptr所指的值为新值。否则，什么也不做
int CompareAndSwap(int *ptr, int expected, int new) {
  int actual = *ptr;
  if (actual == expected)
    *ptr = new;
  return actual;
}

void lock(lock_t *lock) {
  while (CompareAndSwap(&lock->flag, 0, 1) == 1)
    ; // spin
}

//如何创建建C可调用的x86版本的比较并交换，下面的代码段可能有用
char CompareAndSwap(int *ptr, int old, int new) {
  unsigned char ret;

  // Note that sete sets a 'byte' not the word
  __asm__ __volatile__ (
    " lock\n"
    " cmpxchgl %2,%1\n"
    " sete %0\n"
    : "=q" (ret), "=m" (*ptr)
    : "r" (new), "m" (*ptr), "a" (old)
    : "memory");
  return ret;
}