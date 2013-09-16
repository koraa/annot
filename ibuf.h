#ifndef ibuf_h
#define ibuf_h

/**
 * A buffer that knows,
 * how much space is allocated
 * and used.
 * TODO: Distinguish between string and generic buffer in functions.
 */
typedef struct IBuf__ IBuf;
struct IBuf__ {
  size_t len, alloc;
  void *buf;
};
const static size_t itelbuf_stdsize = 512;

#ifndef ibuf_c
extern IBuf *newIBuf(size_t sz);
extern void deleteIBuf(IBuf *b);

extern void ibuf_init(IBuf *nu, size_t sz);
extern void ibuf_destroy(IBuf *b);
extern void ibuf_realloc(IBuf *b, size_t sz);
extern void ibuf_append(IBuf *b, char* s, size_t len);
extern size_t ibuf_app0(IBuf  *b, char* s);
extern void ibuf_drop(IBuf *b, size_t effec);

extern LL *recyclr;
extern IBuf* getfrbuf();
extern void rcyclbuf(IBuf *ibuf);
#endif

#endif
