#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "vector.h"

VECTOR_GENERATE_TYPE_NAME(double, DoubleArray, double_array);
VECTOR_GENERATE_TYPE_NAME_IMPL(double, DoubleArray, double_array);

VECTOR_GENERATE_TYPE_NAME(uint8_t, Buffer, buffer);
VECTOR_GENERATE_TYPE_NAME_IMPL(uint8_t, Buffer, buffer);

int main() {
  DoubleArray *vec = make_double_array();
  assert(double_array_len(vec) == 0);
  for (int i = 0; i < 256; ++i)
    double_array_append(vec, i);
  assert(double_array_len(vec) == 256);

  for (int i = 255; i >= 0; --i)
    double_array_delete(vec, i);
  assert(vec->cap == 32);
  assert(double_array_len(vec) == 0);

  Buffer *vec_buff = make_buffer();
  assert(buffer_len(vec_buff) == 0);
  buffer_append(vec_buff, 'a');
  buffer_append(vec_buff, 'b');
  buffer_append(vec_buff, 'c');
  buffer_append(vec_buff, '\0');
  assert(strcmp((char *)buffer_data(vec_buff), "abc") == 0);
}
