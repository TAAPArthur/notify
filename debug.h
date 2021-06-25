#ifndef NOTIFY_DEBUG_H
#define NOTIFY_DEBUG_H


#ifdef DEBUG
#include <stdio.h>
#define VERBOSE(X...) do {printf("%d: ", getpid()); printf(X);}while(0)
#else
#define VERBOSE(X...)
#endif
#endif

