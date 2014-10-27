/*
 * Copyright (c) 2014 Martin Svanberg, All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE
 */

#ifndef __SAN_H
#define __SAN_H

#define SAN_VERSION_MAJOR 0
#define SAN_VERSION_MINOR 1
#define SAN_VERSION_PATCH 0

#define SAN_OK     0
#define SAN_FAIL  -1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define SAN_DEBUG 1

#if SAN_DEBUG == 1
#define san_dbg(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)

static void *san_dbg_allocptr;
#define SAN_MALLOC(size) (san_dbg_allocptr = malloc(size)); do { san_dbg("MALLOC 0x%x, FILE: %s, LINE: %d\n", (int)san_dbg_allocptr, __FILE__, __LINE__); } while(0)
#define SAN_CALLOC(n, size) (san_dbg_allocptr = calloc(n, size)); do { san_dbg("CALLOC 0x%x, FILE: %s, LINE: %d\n", (int)san_dbg_allocptr, __FILE__, __LINE__); } while(0)
#define SAN_FREE(ptr) free(ptr); do { san_dbg("FREEING 0x%x, FILE: %s, LINE: %d\n", (int)ptr, __FILE__, __LINE__); } while(0)

#else

/* Avoid compiler warnings about unused arguments */
void __san_noop(int k, ...);
#define san_dbg(...) do { __san_noop(0, __VA_ARGS__); } while(0)

#define SAN_FREE(ptr) free(ptr)

#endif

#endif
