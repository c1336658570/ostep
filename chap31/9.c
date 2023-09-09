//一种方案：破除依赖。解决8.c的死锁问题
//假定哲学家4（编写最大的一个）取餐叉的顺序不同
//最后一个哲学家会尝试先拿右手边的餐叉，然后拿左手边，所以不会出现每个哲学家都拿着一个餐叉，卡住等待另一个的情况，等待循环被打破了。

#include <semaphore.h>

//一些辅助函数
int left(int p) { return p; }
int right(int p) { return (p + 1) % 5; }

sem_t forks[5];

void getforks() {
  if (p == 4) {
    sem_wait(forks[right(p)]);
    sem_wait(forks[left(p)]);
  } else {
    sem_wait(forks[left(p)]);
    sem_wait(forks[right(p)]);
  }
}