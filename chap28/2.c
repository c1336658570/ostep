//Dekker的算法（Dekker’s algorithm）只使用了load和store（早期的硬件上，它们是原子的）。实现锁
//Peterson后来改进了Dekker的方法[P81]。同样只使用load和store，保证不会有两个线程同时进入临界区。
//以下是Peterson算法（Peterson’s algorithm，针对两个线程）

int flag[2];
int turn;

//早期通过开关中断实现锁

void init() {
  flag[0] = flag[1] = 0;      // 1->thread wants to grab lock
  turn = 0;                   // whose turn? (thread 0 or 1?)
}
void lock() {
  flag[self] = 1;             // self: thread ID of caller
  turn = 1 - self;            // make it other thread's turn
  while ((flag[1-self] == 1) && (turn == 1 - self))
      ; // spin-wait
}
void unlock() {
  flag[self] = 0;             // simply undo your intent
}

