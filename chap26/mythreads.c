#include "mythreads.h"
#include <assert.h>

//封装了线程创建和合并例程，以便在失败时退出

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  int rc = pthread_create(thread, attr, start_routine, arg);
  assert(rc == 0);
}

int Pthread_join(pthread_t thread, void **retval) {
  int rc = pthread_join(thread, retval);
  assert(rc == 0);
}

int Pthread_mutex_lock(pthread_mutex_t *mutex) {
  int rc = pthread_mutex_lock(mutex);
  assert(rc == 0);
}

int Pthread_mutex_unlock(pthread_mutex_t *mutex) {
  int rc = pthread_mutex_unlock(mutex);
  assert(rc == 0);
}

int Pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m) {
  int rc = pthread_cond_wait(c, m);
  assert(rc == 0);
}
int Pthread_cond_signal(pthread_cond_t *c) {
  int rc = pthread_cond_signal(c);
  assert(rc == 0);
}