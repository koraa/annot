# AnnoT

## NAME

annot - Annotate stdin with timestamps

## INVOKE

annot 

## DESCRIPTIION

Each time a line is red from stdin 

## VERION

AnnoT version 0.0.1b from 09/15/2013.

## AUTHOR

defer.sh has been written by Karolin Varner <karo@cupdev.net>.

## REPORTING BUGS

You can file bug reports and feature requests on github:

https://github.com/koraa/annot/pulls

## COPYING

Copyright © 2013 Free Software Foundation, Inc. License GPLv3+: 
GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

# Hacking

## TODO

* DOC!
* Create manpage
* Error-proof all the types
* TESTS!
* Add writer and reciever to queue
* Define recycle-queue in main
* Options seconds/millis/nanos
* Options include/exclude epoch/since-start/since-last
* Add formatting shell script
* Clang support
* Install support
* Autoinclude headers
* Options custom seperator/zero
* SIGSEGV error message
* Define sbuf better (this '\0' delim is stupid)
* SIGNAL timing support
* Option print-delim/Print each dataset in seperate line
* Option seperator times / data
* Option ignore data/char as just a signal.

## Source

The source code is stored at https://github.com/koraa/annot,
you can issue pull requests for your changes there.
You will requrie Git.

## Codeing style

Generally try to keep consistent with the existing code (duh),
this generally includes indendation and braces,
still here are some *guideliines*:

* Indendation is 2 spaces
* Long statements (e.g. fprintf) should be broken into
  multiple lines. In this case you should double-indent
  the succeeding lines. Make shure ou group the args into
  logic sets then. Don't nest this!
* Avoid more than four levels of indendation
* One statement per line
  * An exception is highly repetitive code,
    in this case you should align your code
    in columns for easy scanning.
    (e.g. in switch/case)
* Omittig braces where possible is OK,
  but keep consistent about braces in this statements
  (e.g. if)
* Lines should generally be no longer than 60 lines
* Return, is like GOTO and breaks the control flow,
  try to use a single one at the end of the function.
  * Same is true for bug()/errxit(), exit(), continue and break
* Files should be no longer than 200 lines of code
* Headers and implementation should be paired or be standalone
* Directories should have no more than 15-20 files
* Do nest deeper than 2 directories
* Directories should provide a meta-header.
* Avoid the preprocessor
* Implicit is fine, if it's  well documented
* If it's broken, let iit crash
* Programmer time is more expensive than compile time and run time
* Don't hide code. We're ale grown ups here, are we mot?

Its OK if your code sometimes breaks these guideline onece in a while
(If you have a good reason, you should even add it to this list),
but try to compartmentalize your code:
If the function gets to long, nested or complicated, create onother one.
If the file gets to long add oneother one.
If the dir has too many files, add oneother one.
If the project get's to complex, create a lib.
*Btw: If you stick to the rules above,
the project can not get longer than 
36k lines of code. This intentional* 

Some ideas are drawen from the Python Zen: http://www.python.org/dev/peps/pep-0020/

## Types

This is a multithreaded program and lots of the syncronisation
uses constructs like linked lists, locks and mutexes.
I am thinking of theis constructs in an Object-Oriented manner
and therefore emmulate it:

* Each such type has it's own header/implementation combination,
  the names of those files should be `$lowercase.c/h`
* They provide a struct that holds the variables.
  Its name is `struct $CamelCase__`, for each of those a
  `typedef struct $CamelCase__ $CamelCase` is provided to
  get rid of the struct.
  * The struct shoud not be directly accessed
  * This struct is defined in the header and included in the
    source file and to everyone who uses that file.
* All methods follow a naming scheme like `$return $lowercase_$fuction($self, ...)`
* All methods take  the struct as their first arg
* There are `void $lowercase_init($self, ...)` and `void $lowercase_destroy($self, ...)` methods that
  initialize/finalize the struct
* There are also `$self new$CamelCase(...)` and `void delete$CamelCase($self);`
  that call the destroy/initr functions and allocate/free space for the struct.

## Errors

Errors are handles by making a call to the functions in err.h,
which will print an error message, their stack trace and exit(i>0).
The stack trace is printed without allocation memory on the heap
and can therefore be employd in case of memory corruption (i.e. SIGSEGV).

This crashing strategy is used, because in this program, most errors will
mean 'something is seriously wrong'.
In cases where errors can effectively be cought, this should of course be done.

Active error checking forstructures like the LL should be employed.

One main reason for wrapping the mutexes (and other types) was creating an abstraction
for this auto-crash behaviour.

## Headers

Each source file shall define a macro `#define $filename_c` on it's first line,
each header shall do the same, but include a test if it's allready defined, to 
allow redundant includes.
Each source file shall include it's own header.
Each header shall handle all the includes for its source file and might declare a
source-file only, externals only and all section.
Please note that this cconstruct does _not_ allow crossover includes.

**Source file `fuu.c`:**

```C
#define fuu_c
#include "fuu.h"

// Here goes all function implentations
```

**Header file `fuu.h`:**
```C
#ifndef fuu_h
#define fuu_h

// BOTH-section
// Includes, macros, types

#ifndef fuu_c
// EXTERNS-Section
// Contains exports
#endif

#ifdef fuu_c
// For fuu.c only
#endif

#endif
```

## Doc

Most documentation should be done javadoc-style in the header,
(i.e. Multiline comments with two stars: `/** ... */`).
This documentation should be mostly about the interface
(functions/macros/types: Purpose, usage, args, return value, effect).
Comments about the indendation can of course be done inline.
