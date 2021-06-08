#ifndef NOTIFY_DEBUG_H
#define NOTIFY_DEBUG_H


#ifdef DEBUG
#include <stdio.h>
#define VERBOSE(X...) printf(X)
#else
#define VERBOSE(X...)
#endif
#endif

