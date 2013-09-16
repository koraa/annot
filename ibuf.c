#define ibuf_c
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "err.h"
#include "mutex.h"
#include "cond.h"
#include "linkedlist.h"
#include "types.h"

#include "ibuf.h"

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
