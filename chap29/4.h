#ifndef LIST__H
#define LIST__H

#include <pthread.h>

//basic node structure
typedef struct node_t {
  int key;
  struct node_t *next;
} node_t;

// basic list structure (one used per list)
typedef struct list_t {
  node_t *head;
  pthread_mutex_t lock;
} list_t;

#endif