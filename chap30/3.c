//通过条件变量实现输出
//parent: begin 
//child
//parent: end

//gcc 3.c ../chap26/mythreads.c -o 3 -I../chap26/

#include <pthread.h>
#include <stdio.h>
#include "mythreads.h"

int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void thr_exit() {
  Pthread_mutex_lock(&m);
  done = 1;
  Pthread_cond_signal(&c);
  Pthread_mutex_unlock(&m);
}

void *child(void *arg) {
  printf("child\n");
  thr_exit();
  return NULL;
}

void thr_join() {
  Pthread_mutex_lock(&m);
  while (done == 0) Pthread_cond_wait(&c, &m);
  Pthread_mutex_unlock(&m);
}

int main(int argc, char *argv[]) {
  printf("parent: begin\n");
  pthread_t p;
  Pthread_create(&p, NULL, child, NULL);
  thr_join();
  printf("parent: end\n");
  return 0;
}


//如下代码是否正确？
//错误的
//假设子线程立刻运行，并且调用thr_exit()。在这种情况下，子线程发送信号，但此时却没有在条件变量上睡眠等待的线程。
//父线程运行时，就会调用wait并卡在那里，没有其他线程会唤醒它。
void thr_exit() {
  Pthread_mutex_lock(&m);
  Pthread_cond_signal(&c);
  Pthread_mutex_unlock(&m);
}

void thr_join() {
  Pthread_mutex_lock(&m);
  Pthread_cond_wait(&c, &m);
  Pthread_mutex_unlock(&m);
}


//另一个糟糕的实现
//这里的问题是一个微妙的竞态条件。具体来说，如果父进程调用thr_join()，然后检查完done的值为0，然后试图睡眠。
//但在调用wait进入睡眠之前，父进程被中断。子线程修改变量done为1，发出信号，同样没有等待线程。父线程再次运行时，就会长眠不醒。
void thr_exit() {
  done = 1;
  Pthread_cond_signal(&c);
}

void thr_join() {
  if (done == 0) Pthread_cond_wait(&c);
}