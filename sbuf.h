#ifndef sbuf_h
#define sbuf_h

#ifndef sbuf_c
extern void fjoin(FILE *f);
extern size_t getsep(FILE *f, char delim, size_t size, char *buf);

extern void getItok(FILE *f, char delim, LL *qu);
extern void putItok(FILE *f, LL *tok);
#endif

#endif
