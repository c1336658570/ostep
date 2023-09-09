// 尝试用一个共享变量，完成1的预期输出
//效率低下，因为主线程会自旋检查，浪费CPU时间

//gcc 2.c ../chap26/mythreads.c -o 2 -I../chap26/ 

#include <stdio.h>
#include "mythreads.h"

volatile int done = 0;

void *child(void *arg) {
  printf("child\n");
  done = 1;
  return NULL;
}

int main(int argc, char *argv[]) {
  printf("parent: begin\n");
  pthread_t c;
  Pthread_create(&c, NULL, child, NULL);  // create child
  while (done == 0)
    ;  // spin
  printf("parent: end\n");
  return 0;
}