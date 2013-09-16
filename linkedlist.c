#define linkedlist_c
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "types.h"
#include "mutex.h"
#include "cond.h"
#include "err.h"

#include "linkedlist.h"

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
