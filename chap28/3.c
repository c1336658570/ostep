//Dekker算法，软件实现锁

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define true 1
#define false 0

typedef int bool;
bool flag[2];
int turn;

void visit(int num) {
  sleep(1);
  printf("P%d is visting\n", num);
}

void P0() {
  while (true) {
    flag[0] = true;  // P0想使用关键区。
    while (flag[1])  // 检查P1是不是也想用？
    {
      if (turn == 1)  // 如果P1想用，则查看P1是否具有访问权限？
      {
        flag[0] = false;  // 如果有，则P0放弃。
        while (turn == 1)
          ;              // 检查turn是否属于P1。
        flag[0] = true;  // P0想使用。
      }
    }
    visit(0);         // 访问Critical Partition。
    turn = 1;         // 访问完成，将权限给P1。
    flag[0] = false;  // P0结束使用。
  }
}

void P1() {
  while (true) {
    flag[1] = true;  // P1想使用关键区。
    while (flag[0])  // 检查P0是不是也想用？
    {
      if (turn == 0)  // 如果P0想用，则查看P0是否具有访问权限？
      {
        flag[1] = false;  // 如果有，则P1放弃。
        while (turn == 0)
          ;              // 检查turn是否属于P1。
        flag[1] = true;  // P1想使用。
      }
    }
    visit(1);         // 访问Critical Partition。
    turn = 0;         // 访问完成，将权限给P0。
    flag[1] = false;  // P1结束使用。
  }
}

void main() {
  pthread_t t1, t2;
  flag[0] = flag[1] = false;
  turn = 0;
  int err;
  err = pthread_create(&t1, NULL, (void*)P0, NULL);
  if (err != 0) exit(-1);
  err = pthread_create(&t2, NULL, (void*)P1, NULL);
  if (err != 0) exit(-1);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  exit(0);
}