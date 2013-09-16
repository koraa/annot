#define sbuf_c
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "mutex.h"
#include "cond.h"
#include "err.h"
#include "stdlib.h"
#include "linkedlist.h"

#include "ibuf.h"
#include "sbuf.h"

/**
 * Wait for the given stream to be readable.
 */
void fjoin(FILE *f) {
  int c;
  c = fgetc(f);
  ungetc(c, f);
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


