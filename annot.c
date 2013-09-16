#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "err.h"
#include "types.h"
#include "thread.h"
#include "mutex.h"
#include "cond.h"
#include "nlock.h"
#include "linkedlist.h"
#include "ibuf.h"
#include "sbuf.h"
#include "queue.h"

// Each of these queues is actually
// a linked list.
// Besides thati, we also use a linked
// list for each buffer, so that each
// n-byte string is liked to the next.
//
// Those string buffers are not being deallocated in
// the end. We just put them on the recyclr for reuse.
LL *inQ, *timeQ;

typedef struct ArgRead__ ArgRead;
struct ArgRead__ {
  char delim;
  FILE *F;
};
void *thr_read(void *arg__) {
  ArgRead *arg = arg__;

  Tok *t; 
  while (feof(arg->F)==0) { // Each token (mostly lines)
    t = newTok();
    nlock_lock(t->edit);

    fjoin(arg->F);   // Wait for the stream to have data
    ll_push(inQ, t); // Pass on for timestamp

    // Now read the data
    getItok(arg->F, arg->delim, t->str);
    nlock_unlock(t->edit);
  }
 
  inQ->EOT = true;   
  return 0;
}

// TODO: Semaphore write
// TODO: Wait for  add something in pop()

tstamp nanotime(clockid_t clock) {
  struct timespec ts;
  clock_gettime(clock, &ts);
  return ts.tv_sec *sec
       + ts.tv_nsec *nano; 
}

tstamp nanoepoch() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * sec
       + tv.tv_usec * micro;
}

typedef struct ArgTime__ ArgTime;
struct ArgTime__ {
  clockid_t clock;
};
void *thr_time(void *arg__) {
  ArgTime *arg= arg__;

  tstamp 
    t0 = nanotime(arg->clock),
    t0epoch = nanoepoch(),
    lastt = t0,
    curt;
  Tok *t;

  while (!inQ->EOT) {
    t = ll_pop(inQ);

    nlock_lock(t->edit);
    ll_push(timeQ, t);
  
    curt = nanotime(arg->clock); 
    t->sstart = curt - t0;
    t->slast  = curt - lastt;
    t->epoch  = t->sstart - t0epoch;

    nlock_unlock(t->edit);

    lastt = curt;
  }

  timeQ->EOT = true;

  return 0;
}

typedef struct ArgPrint__ ArgPrint;
struct ArgPrint__ {
  char delim;
  FILE *f;
};
void *thr_print(void *arg__) {
  ArgPrint *args = arg__;
  
  Tok *t;
  while (!inQ->EOT) {
    t = ll_pop(timeQ);
    nlock_wait(t->edit);

    fprintf(args->f,
        "%" PRIu64 " %" PRIu64 " %" PRIu64 " ", 
        t->epoch, t->sstart, t->slast);
    putItok(args->f, t->str); 
    fputc(args->delim, args->f);

    deleteTok(t);
  }

  return 0;
}

int main(int argc, char** argv) {
  recyclr = newLL();
  inQ = newLL();
  timeQ = newLL();

  ArgRead  ar = {'\n', stdin};
  ArgTime  at = {CLOCK_MONOTONIC};
  ArgPrint ap = {'\n', stdout};

  // Run the program

  Thread
    *tr = newThread(NULL, thr_read, &ar),
    *tt = newThread(NULL, thr_time, &at);
  thr_print(&ap);

  // We exit soon, still run deletion,
  // to clean up possible FDs and do some
  // error checking

  deleteThread(tr);
  deleteThread(tt); 

  deleteLL(recyclr);
  deleteLL(inQ);
  deleteLL(timeQ);

  return 0;
}
