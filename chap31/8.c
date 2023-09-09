//哲学家就餐问题

//每个哲学家的基本循环：
/*
while (1) {
  think(); 
  getforks(); 
  eat(); 
  putforks();
}
*/

#include <semaphore.h>

//一些辅助函数
int left(int p) { return p; }
int right(int p) { return (p + 1) % 5; }

//有问题的解决方案
//存在死锁
//假设每个哲学家都拿到了左手边的餐叉，他们每个都会阻塞住，并且一直等待另一个餐叉。具体来说，哲学家0拿到了餐叉0，
//哲学家1拿到了餐叉1，哲学家2拿到餐叉2，哲学家3拿到餐叉3，哲学家4拿到餐叉4。所有的餐叉都被占有了，所有的哲学家都阻塞着，
//并且等待另一个哲学家占有的餐叉。
sem_t forks[5];

void getforks() {
  sem_wait(forks[left(p)]);
  sem_wait(forks[right(p)]);
}

void putforks() {
  sem_post(forks[left(p)]);
  sem_post(forks[right(p)]);
}