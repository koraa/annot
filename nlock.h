#ifndef nlock_h
#define nlock_h

//#include <pthread.h>
//#include "mutex.h"
//#include "cond.h"

typedef struct NLock__ NLock;
struct NLock__ {
  Mutex *m;
  Cond  *c;
  unsigned int i;
};

#ifndef nlock_c
extern NLock *newNLock();
extern void deleteNLock(NLock *m);

extern void nlock_init(NLock *m);
extern void nlock_destroy(NLock *m);
extern void nlock_lock(NLock *m);
extern void nlock_unlock(NLock *m);
extern void nlock_wait(NLock *m);
#endif

#endif
