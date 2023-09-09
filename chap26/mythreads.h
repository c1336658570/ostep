#ifndef MYTHREADS__H
#define MYTHREADS__H

#include <pthread.h>

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int Pthread_join(pthread_t thread, void **retval);
int Pthread_mutex_lock(pthread_mutex_t *mutex);
int Pthread_mutex_unlock(pthread_mutex_t *mutex);
int Pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
int Pthread_cond_signal(pthread_cond_t *c);

#endif