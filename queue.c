#define queue_c
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "mutex.h"
#include "cond.h"
#include "linkedlist.h"
#include "nlock.h"
#include "types.h"

#include "queue.h"

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

