#ifndef thread_h
#define thread_h

// #include <pthread.h>

typedef pthread_t Thread;

#ifndef thread_c
extern Thread *newThread(
    const pthread_attr_t *attr,
    void *(*code) (void *),
    void *arg);
extern void deleteThread(Thread *t);
#endif

#endif
