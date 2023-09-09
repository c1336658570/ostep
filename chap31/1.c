//信号量的定义
#include <semaphore.h>

sem_t s;
sem_init(&s, 0, 1);

//信号量初始化之后，可以调用sem_wait()或sem_post()与之交互
int sem_wait(sem_t *s) {
  decrement the value of semaphore s by one 
  wait if value of semaphore s is negative
}

int sem_post(sem_t *s) {
  increment the value of semaphore s by one 
  if there are one ormore threads waiting,wake one
}