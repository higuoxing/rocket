#include "vector.h"

static void vector_init(Vector *vec) {
  vec->cap = VECTOR_DEFAULT_INIT_SIZE;
  vec->len = 0;
  vec->items = (void **)malloc(VECTOR_DEFAULT_INIT_SIZE * sizeof(void *));
}

Vector *make_vector(void) {
  Vector *vec = (Vector *)malloc(sizeof(Vector));
  vector_init(vec);
  return vec;
}

int vector_len(Vector *vec) {
  return vec->len;
}

void *vector_get(Vector *vec, int index) {
  if (index < 0 || index >= vec->len)
    return NULL;
  return vec->items[index];
}

bool vector_set(Vector *vec, int index, void *item) {
  if (index < 0 || index >= vec->len)
    return false;
  vec->items[index] = item;
  return true;
}

static void vector_resize(Vector *vec, int new_cap) {
  void **items = realloc(vec->items, new_cap);
  if (items) {
    vec->items = items;
    vec->cap = new_cap;
  }
}

void vector_append(Vector *vec, void *item) {
  if (vec->len == vec->cap)
    vector_resize(vec, vec->cap * 2);
  vec->items[vec->len++] = item;
}

void vector_delete(Vector *vec, int index) {
  if (index < 0 || index > vec->len)
    return;

  vec->items[index] = NULL;

  for (int i = index; i < vec->len; ++i) {
    vec->items[i] = vec->items[i + 1];
    vec->items[i + 1] = NULL;
  }

  vec->len -= 1;

  if (vec->len > 0 && vec->len == vec->cap / 4)
    vector_resize(vec, vec->cap / 2);
}

void free_vector(Vector *vec) {
  vec->len = 0;
  vec->cap = 0;
  free(vec->items);
  vec->items = NULL;
}
