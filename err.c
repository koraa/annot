#define err_c

#include <stdio.h>
#include <sys/types.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdarg.h>

#include "err.h"

void printBacktrace(FILE* f) {
  size_t stacksp = 1024;
  void *stack[stacksp];
  
  size_t stackl = backtrace(stack, stacksp);
  fputs("STACK TRACE:\n", f);
  backtrace_symbols_fd(stack, stackl, fileno(f));
  fputc('\n', stderr);
}

void msgxit__(int no, char* annot, char* fmt, va_list args) {
  // TODO: Print stack trace

  fprintf(stderr, "[%s] ", annot);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  printBacktrace(stderr);
  fprintf(stderr, "Sorry, bye: exit(%i)\n", no);

  exit(no);
}

void bug__(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  msgxit(2, "FATAL BUG", fmt, args);

  va_end(args);
}

void er_args__(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  msgxit(1, "ERROR: ARGS", fmt, args);

  va_end(args);
}
