#ifndef linkedlist_h
#define linkedlist_h

//#include <pthread.h>
//#include <stdbool.h>
//#include "mutex.h"
//#include "cond.h"

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

#ifndef linkedlist_c
extern LL* newLL();
extern void deleteLL(LL* ll);

extern void ll_init(LL* ll);
extern void ll_destroy(LL *ll);
extern bool ll_empty(LL *ll);
extern void ll_waitr(LL *l);
extern void ll_push(LL *queue, void* buf);
extern void* ll_pop(LL *queue);
extern int ll_trypop(LL *l, void **buf);
#endif

#endif
