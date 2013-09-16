#define nlock_c
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "mutex.h"
#include "cond.h"
#include "types.h"

#include "nlock.h"

void nlock_init(NLock *m) {
  m->m = newMutex(NULL);
  m->c = newCond(NULL);
  m->i = 0;
}

void nlock_destroy(NLock *m) {
  deleteMutex(m->m);
  deleteCond(m->c);
}

NLock *newNLock() {
  NLock *nu = talloc(NLock);
  nlock_init(nu);
  return nu;
}

void deleteNLock(NLock *m) {
  nlock_destroy(m);
  free(m);
}

void nlock_lock(NLock *m) {
  mutex_lock(m->m);
  m->i++;
  mutex_unlock(m->m);
}

void nlock_unlock(NLock *m) {
  mutex_lock(m->m);
  m->i--;
  mutex_unlock(m->m);

  if (m->i==0) // This is redundant. Actual check in wait
    cond_signal(m->c);
}

void nlock_wait(NLock *m) {
  mutex_lock(m->m);
  while (true) {
    if (m->i == 0)
      break;
    cond_wait(m->c, m->m);
  }
  mutex_unlock(m->m);
}

