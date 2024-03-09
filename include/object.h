#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "common.h"

typedef enum ObjectType {
  OBJ_NIL = 0,
  OBJ_BOOL,
  OBJ_NUMBER,
} ObjectType;

typedef struct Object {
  ObjectType type;
  Datum value;
} Object;


#endif
