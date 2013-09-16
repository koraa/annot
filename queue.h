#ifndef queue_h
#define queue_h

/**
 * Token - The unit used in the processing Queue
 */
typedef struct Tok__ Tok;
struct Tok__ {
  tstamp epoch, sstart, slast;
  LL *str; // Use not thread save LL?
  NLock *edit;
};


#ifndef queue_c
extern Tok* newTok();
extern void tok_destroy(Tok *t);

extern void tok_init(Tok *nu);
extern void deleteTok(Tok *t);
#endif

#endif
