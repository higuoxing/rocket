#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "common.h"

#define VECTOR_DEFAULT_INIT_SIZE 32

typedef struct Vector {
  int cap;
  int len;
  Datum *items;
} Vector;

extern Vector *make_vector(void);
extern int vector_len(Vector *vec);
extern Datum *vector_data(Vector *vec);
extern Datum vector_get(Vector *vec, int index);
extern bool vector_set(Vector *vec, int index, Datum item);
extern void vector_append(Vector *vec, Datum item);
extern void vector_delete(Vector *vec, int index);
extern void free_vector(Vector *vec);

#endif /* _VECTOR_H_ */
