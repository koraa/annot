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

typedef pthread_cond_t Cond;

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

typedef struct NLock__ NLock;
struct NLock__ {
  Mutex *m;
  Cond  *c;
  unsigned int i;
};

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

typedef struct LLelem__ LLelem;
struct LLelem__ {
  void *buf;
  LLelem \
    *next,
    *prev; 
};

typedef struct LL__ LL;
struct LL__ {
  LLelem \
    *head, 
    *last;
  Cond *sigavail;
  Mutex *lock; 
  bool EOT;  
};

bool ll_empty(LL *ll) {
  bool R = ll->head == NULL;
  if ((ll->last == NULL) != R)
    bug("Illegal ll state: %p [%p, %p]",
        ll, ll->head, ll->last);
  return R; 
}

void ll_init(LL* ll) {
  ll->head = ll->last = NULL;
  ll->lock = newMutex(NULL);
  ll->sigavail = newCond(NULL);
  ll->EOT = false;
}

void ll_destroy(LL *ll) {
  if (!ll_empty(ll))
    bug("Trying to destroy LL that still has elements (%p)", ll);

  deleteMutex(ll->lock);
}

LL* newLL() {
  LL *nu = talloc(LL);
  ll_init(nu);
  return nu;
}

void deleteLL(LL* ll) {
  ll_destroy(ll);
  free(ll);
}

void ll_waitr(LL *l) {
  // TODO: Recursive lock
  while (true) {
    if (!ll_empty(l))
      return;
    cond_wait(l->sigavail, l->lock);
  }
}

void ll_push(LL *queue, void* buf) {
  mutex_lock(queue->lock);

  LLelem *nlle = talloc(LLelem);
  nlle->buf  = buf;
  nlle->next = queue->head;
  nlle->prev = NULL;

  queue->head = nlle;
  if (queue->last == NULL) 
    queue->last = nlle;

  mutex_unlock(queue->lock);
  cond_signal(queue->sigavail);
}

/**
 * Pop the last element from the given queue and return it's buffer.
 * Returns NULL if there are no free elements
 */
void* ll_pop(LL *queue) {
  mutex_lock(queue->lock); 
  ll_waitr(queue);
  cond_signal(queue->sigavail);


  LLelem *nend = queue->last->prev;
  void* R = queue->last->buf;

  free(queue->last);

  if (nend==NULL) {
    queue->head = queue->last = NULL;
  } else {
    nend->next = NULL;
    queue->last = nend;
  }

  mutex_unlock(queue->lock);
  return R;

}

int ll_trypop(LL *l, void **buf) {
  mutex_lock(l->lock);
  bool em = ll_empty(l);
  mutex_unlock(l->lock);

  if (em)
    return false;
  else {
    *buf = ll_pop(l);
    return true;
  }
}

/**
 * A buffer that knows,
 * how much space is allocated
 * and used.
 * TODO: Distinguish between string and generic buffer in functions.
 */
typedef struct IBuf__ IBuf;
struct IBuf__ {
  size_t len, alloc;
  void *buf;
};
const static size_t itelbuf_stdsize = 512;

void ibuf_init(IBuf *nu, size_t sz) {
  if (sz < 1)
    bug("Trying to initialize IBuf with size<1.");

  nu->alloc = sz;
  nu->buf = malloc(sz);

  nu->len = 1;
  ((char*) nu->buf)[0] =  '\0';
}
IBuf *newIBuf(size_t sz) {
  IBuf *nu = talloc(IBuf);
  ibuf_init(nu, sz);
  return nu;
}

void ibuf_destroy(IBuf *b) {
  free(b->buf);
  b->len = b->alloc = 0;
  b->buf = NULL;
}

void deleteIBuf(IBuf *b) {
  ibuf_destroy(b);
  free(b);
}

void ibuf_realloc(IBuf *b, size_t sz) {
  if (sz <  b->len)
    bug("Trying to resize buffer[%p] from %lib to %lib"
        "while it's length(=%li) is smaller than the new alloc.",
        b, b->alloc, sz, b->len);
}

void ibuf_append(IBuf *b, char* s, size_t len) {
  if (len == 0)
    return;

  size_t sz = b->len + len;
  if (sz > b->alloc)
    ibuf_realloc(b, sz);

  memcpy(b + b->len - 1, s, len);
  ((char*) b->buf)[sz] = '\0';

  b->len = sz;
}

size_t ibuf_app0(IBuf  *b, char* s) {
  size_t sz = strlen(s);
  ibuf_append(b,s,sz);
  return sz;
}

void ibuf_drop(IBuf *b, size_t effec) {
  b->len = effec+1;
  ((char*) b->buf)[b->len-1]  = '\0';
}

// We store the buffers to recycle here*/
LL *recyclr;

/** Reuse a recycled buffer or generate a new one if no buffer is available */
IBuf* getfrbuf() {
  IBuf *ibuf;
  if (!ll_trypop(recyclr, (void*)&ibuf)) {
    ibuf = newIBuf(itelbuf_stdsize);
  }
  ibuf->len = 0;
  return ibuf;
}


/*
 * Recycle a buffer when not needing it any more.
 */
void rcyclbuf(IBuf *ibuf) {
  ll_push(recyclr, ibuf);
}

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
