#ifndef cond_h
#define cond_h

//#include <pthread.h>

typedef pthread_cond_t Cond;

#ifndef cond_c
extern Cond* newCond(const pthread_condattr_t *attr);
extern void deleteCond(Cond *c);

extern void cond_signal(Cond *c);
extern void cond_broadcast(Cond *c);
extern void cond_wait(Cond *c, Mutex *m);
#endif

#endif
