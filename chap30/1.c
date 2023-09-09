// 在很多情况下，线程需要检查某一条件（condition）满足之后，才会继续运行。例如，父线程需要检查子线程是否执行完毕
// [这常被称为join()]。

//gcc 1.c ../chap26/mythreads.c -o 1 -I../chap26/ 

//期望如下输出，但是真实的输出结果不确定
//parent: begin 
//child
//parent: end

#include "mythreads.h"
#include <stdio.h>

void *child(void *arg) {
  printf("child\n");
  // XXX how to indicate we are done?
  return NULL;
}

int main(int argc, char *argv[]) {
  printf("parent: begin\n");
  pthread_t c;
  Pthread_create(&c, NULL, child, NULL);  // create child
  // XXX how to wait for child?
  printf("parent: end\n");
  return 0;
}