//Peterson算法，软件实现锁

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define true 1
#define false 0

typedef int bool;
bool flag[2];
int turn;

void procedure0() {
  while (true) {
    flag[0] = true;
    turn = 1;
    while (flag[1] && turn == 1)  // 退出while循环的条件就是，要么另一个线程
    // 不想要使用关键区，要么此线程拥有访问权限。
    {
      sleep(1);
      printf("procedure0 is waiting!\n");
    }
    // critical section
    flag[0] = false;
  }
}

void procedure1() {
  while (true) {
    flag[1] = true;
    turn = 0;
    while (flag[0] && turn == 0) {
      sleep(1);
      printf("procedure1 is waiting!\n");
    }
    // critical section
    flag[1] = false;
  }
}

void main() {
  pthread_t t1, t2;
  flag[0] = flag[1] = false;
  int err;
  turn = 0;
  err = pthread_create(&t1, NULL, (void*)procedure0, NULL);
  if (err != 0) exit(-1);
  err = pthread_create(&t2, NULL, (void*)procedure1, NULL);
  if (err != 0) exit(-1);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  exit(0);
}