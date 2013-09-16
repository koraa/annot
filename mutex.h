#ifndef mutex_h
#define mutex_h

// #include <pthread.h>

typedef pthread_mutex_t Mutex;

#ifndef mutex_c
extern Mutex* newMutex(const pthread_mutexattr_t *attr);
extern void deleteMutex(Mutex *m);

extern void mutex_lock(Mutex *m);
extern bool mutex_trylock(Mutex *m);
extern void mutex_unlock(Mutex *m);
#endif

#endif
