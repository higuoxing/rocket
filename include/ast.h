#ifndef _AST_H_
#define _AST_H_

#include "common.h"
#include "vector.h"

typedef enum AstKind {
  AST_BOOL,
  AST_CHAR,
  AST_NUMBER,
  AST_IDENT,
  AST_PROC_CALL,
  AST_CONS,
} AstKind;

typedef struct AstNode {
  AstKind kind;
} AstNode;

typedef struct AstBool {
  AstNode base;
  bool boolean;
} AstBool;

typedef struct AstChar {
  AstNode base;
  char char_;
} AstChar;

typedef struct AstNumber {
  AstNode base;
  double number;
} AstNumber;

typedef struct AstIdent {
  AstNode base;
  char *ident;
} AstIdent;

typedef struct AstProcCall {
  AstNode base;
  AstNode *callable;
  Vector *args;
} AstProcCall;

// extern Cons *make_cons(void *car, void *cdr);
// extern Cons *list_reverse(Cons *list);

extern AstNode *make_ast_bool(bool b);
extern AstNode *make_ast_char(char c);
extern AstNode *make_ast_number(double d);
extern AstNode *make_ast_ident(const char *id);
extern AstNode *make_ast_proc_call(AstNode *callable, Vector *args);
extern void free_ast_node(AstNode *node);

#endif
