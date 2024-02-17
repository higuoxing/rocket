#ifndef _PARSER_H_
#define _PARSER_H_

#include "vector.h"
#include "ast.h"

extern Ast *parse_program(Vector *tokens);

#endif
