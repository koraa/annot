#define cond_c
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mutex.h"
#include "types.h"

#include "cond.h"


Cond* newCond(
    const pthread_condattr_t *attr) {
  Cond *nu = talloc(Cond);   
  pthread_cond_init(nu, attr);
  return nu;
}

void deleteCond(Cond *c) {
  pthread_cond_destroy(c);
  free(c);
}

void cond_signal(Cond *c) {
  // TODO: Error Handling
  pthread_cond_signal(c);
}

void cond_broadcast(Cond *c) {
  // TODO: Error Handling
  pthread_cond_broadcast(c);
}

void cond_wait(Cond *c, Mutex *m) {
  // TODO: Error Handling
  pthread_cond_wait(c,m);
}
