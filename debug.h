#ifndef NOTIFY_DEBUG_H
#define NOTIFY_DEBUG_H


#ifdef DEBUG
#include <stdio.h>
#define VERBOSE(...) do {printf("%d: ", getpid()); printf(__VA_ARGS__);}while(0)
#else
#define VERBOSE(...)
#endif
#endif

