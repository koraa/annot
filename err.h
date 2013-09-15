#ifndef err_h
#define err_h

#define ERRERR 253
#define msgxit(...) { msgxit__(__VA_ARGS__); exit(ERRERR);}
#define bug(...) { bug__(__VA_ARGS__); exit(ERRERR); }
#define er_args(...) {bug(__VA_ARGS__); exit(ERRERR);}

#ifndef err_c
extern void printBacktrace(FILE* f);
extern void msgxit__(int no, char* annot, char* fmt, va_list args);
extern void bug__(char* fmt, ...);
extern void er_args__(char* fmt, ...);
#endif

#endif
