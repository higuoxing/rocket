#ifndef _PARSER_H_
#define _PARSER_H_

#include "ast.h"
#include "tokenizer.h"
#include "vector.h"

extern Vector *parse_program(TokenIter *iter);

#endif
