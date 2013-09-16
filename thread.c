#define thread_c
#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>
#include "types.h"

#include "thread.h"

Thread *newThread(
    const pthread_attr_t *attr,
    void *(*code) (void *),
    void *arg) {
  // Todo: Error handling
  Thread *nu = talloc(Thread);
  pthread_create(nu, attr, code, arg);
  return nu;
}

void deleteThread(Thread *t) {
  // TODO: Check if still running
  free(t);
}
