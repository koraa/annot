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


/**
 * fgets with a custom delimiter
 *
 * Reads up to size bytes from f and stores them in buf.
 * If EOF ore delim is reached the function stops prematurely
 * and stores a '\0' in the end.
 * If EOF is reached right at the first char, buf is not touched
 * and 0 is returned.
 *
 * Effectively:
 *  * Check if a full token was red by buf[$return_value]=='\0'
 *  * Check if EOF was reached by $return_value==0
 *
 * ARGS
 *  f - The file to read from
 *  delim - The delimiter to search for (eg. '\n', '\0', ' ')
 *  size - The size of buf
 *  buf - The buffer to store the bytes red in 
 *
 * RETURNS the number of bytes red.
 */
size_t getsep(FILE *f, char delim, size_t size, char *buf) {
  // TODO: This seems like a fairly naive implementation
  int C; size_t cnt;
  for (cnt=0; cnt < size; cnt++) {
    C = fgetc(f);
    if (C==EOF && cnt==0)
        return -1;
    else if (C==EOF || C==delim) {
        buf[cnt]='\0';
        return cnt+1;// We did write the additional NUL
    } else {
      buf[cnt] = C;
    }
  }
  return cnt; 
}

/**
 * Wait for the given stream to be readable.
 */
void fjoin(FILE *f) {
  int c;
  c = fgetc(f);
  ungetc(c, f);
}

/**
 * Wrapper around getsep, that reads a token from a file 
 * into a linked list of IBufs.
 * while using rcyclbuf/getfrbuf to aquire its buffers.
 */
void getItok(FILE *f, char delim, LL *qu) {
  IBuf *ibuf;
  size_t no;
  while (true) {
    ibuf = getfrbuf();

    no=getsep(stdin, delim, ibuf->alloc, ibuf->buf);
    ibuf->len = no;

    if (no==0)
      rcyclbuf(ibuf);
    else
      ll_push(qu, ibuf);

    if ( ((char*)ibuf->buf)[no-1]=='\0' || no==0)
      return;
  }
}

/**
 * Write a Itok (LL of IBufs) to file.
 * All buffers are stripped of the
 * list and recicled (the list will be empty).
 * The last byte of the last buffer will be omitted,
 * because it should contain a \0.
 *
 * TODO: The \0 delim is stupid.
 */
void putItok(FILE *f, LL *tok) {
  if (ll_empty(tok))
    bug("Trying to write an empty Itok.");

  IBuf *b;
  bool empty=false;
  while (!empty) {
    b = ll_pop(tok);
    empty = ll_empty(tok);

    fwrite(b->buf, 1, b->len - (empty?1:0), f); 

    rcyclbuf(b);
  }
}


/////////////////////////////////////////
// PROGRAM

/**
 * Token - The unit used in the processing Queue
 */
typedef struct Tok__ Tok;
struct Tok__ {
  tstamp epoch, sstart, slast;
  LL *str; // Use not thread save LL?
  NLock *edit;
};

void tok_init(Tok *nu) {
  nu->str = newLL();
  nu->edit = newNLock();
}

Tok* newTok() {
  Tok *nu = talloc(Tok);
  tok_init(nu);
  return nu;
}

void tok_destroy(Tok *t) {
  deleteLL(t->str); 
  deleteNLock(t->edit);
}

void deleteTok(Tok *t) {
  tok_destroy(t);
  free(t);
}

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
