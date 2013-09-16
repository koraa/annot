#define mutex_c
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include "types.h"
#include "err.h"


#include "mutex.h"

// TODO: Handle mutex errors

Mutex* newMutex(const pthread_mutexattr_t *attr) {
  Mutex *nu = talloc(Mutex);
  pthread_mutex_init(nu, attr);
  return nu;
}

void deleteMutex(Mutex *m) {
  int er = pthread_mutex_destroy(m);
  if (er != 0)
    bug("Trying to destroy a mutex failed: %p\n ERRCODE %i", m, er);
  free(m);
}

// TODO: Better
void mutex_lock(Mutex *m) {
  int r = pthread_mutex_lock(m);

  // No error
  if (r==0) return;

  Str er;
  switch (r) {
    case EINVAL:  er = "EINVAL";    break;
    case EAGAIN:  er = "EAGAIN";    break;
    case EDEADLK: er = "EDEADLK";   break;
    default:      er = "[unknown]"; break;
  }

  bug("Error in pthread_mutex_lock (%i - %s). See `man pthread_mutex_trylock`", r, er);
}

bool mutex_trylock(Mutex *m) {
  int r = pthread_mutex_trylock(m);

  // No error
  if (r==0)
    return true;
  else if (r==EBUSY)
    return false;

  Str er;
  switch (r) {
    case EINVAL: er = "EINVAL";    break;
    case EAGAIN: er = "EAGAIN";    break;
    case EPERM:  er = "EPERM";     break;
    default:     er = "[unknown]"; break;
  }

  bug("Error in pthread_mutex_trylock (%i - %s). See `man pthread_mutex_trylock`", r, er);
  exit(199);
}

void mutex_unlock(Mutex *m) {
  int r = pthread_mutex_unlock(m);

  // No error
  if (r==0)
    return;

  Str er;
  switch (r) {
    case EINVAL: er = "EINVAL";    break;
    case EAGAIN: er = "EAGAIN";    break;
    case EPERM:  er = "EPERM";     break;
    default:     er = "[unknown]"; break;
  }

  bug("Error in pthread_mutex_unlock (%i - %s). See `man pthread_mutex_trylock`", r, er);
}
