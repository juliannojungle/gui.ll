#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUGMSGS
    #include <stdio.h>
    #define SHOWDEBUG(...) printf(__VA_ARGS__)
#else
    #define SHOWDEBUG(...) ((void)0)
#endif

#endif /* DEBUG_H */
