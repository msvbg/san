#ifndef __SAN_H
#define __SAN_H

#define SAN_OK     0
#define SAN_FAIL  -1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define SAN_DEBUG 0
#if SAN_DEBUG == 1
#define san_dbg(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
/* Avoid compiler warnings about unused arguments */
void __san_noop(int k, ...);
#define san_dbg(...) do { __san_noop(0, __VA_ARGS__); } while(0)
#endif

#endif
