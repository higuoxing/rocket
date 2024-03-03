#ifndef _COMMON_H_
#define _COMMON_H_

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/chardefs.h>
#include <readline/history.h>
#include <readline/readline.h>

#define BLKSZ 1024

typedef uintptr_t Datum;

#define BoolGetDatum(b) ((Datum)b)
#define Int64GetDatum(i) ((Datum)i)
#define CStringGetDatum(str) ((Datum)str)
#define PointerGetDatum(ptr) ((Datum)ptr)

#define DatumGetBool(d) ((bool)d)
#define DatumGetInt64(d) ((int64_t)d)
#define DatumGetCString(d) ((char *)d)
#define DatumGetPtr(d) ((void *)d)

static inline Datum FloatGetDatum(double f) {
  union {
    double val;
    int64_t retval;
  } myunion;
  myunion.val = f;
  return (Datum)myunion.retval;
}

static inline double DatumGetFloat(Datum d) {
  union {
    int64_t val;
    double retval;
  } myunion;
  myunion.val = DatumGetInt64(d);
  return myunion.retval;
}

#endif
