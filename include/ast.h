#ifndef _AST_H_
#define _AST_H_

#include "vector.h"
#include <stdbool.h>

struct AstNode;

typedef enum AstKind {
  AK_Boolean,
  AK_Char,
  AK_Number,
  AK_Ident,
  AK_ProcCall,
  AK_Cons,
} AstKind;

typedef struct Cons {
  void *car;
  void *cdr;
} Cons;

typedef struct ProcCall {
  struct AstNode *callable;
  Vector *args;
} ProcCall;

typedef union AstVal {
  /* AK_Boolean */
  bool boolean;
  /* AK_Char */
  char char_;
  /* AK_Number */
  double number;
  /* AK_Ident */
  char *ident;
  /* AK_Cons */
  Cons *cons;
  /* AK_ProcCall */
  ProcCall proc_call;
} AstVal;

typedef struct AstNode {
  AstKind kind;
  AstVal val;
} Ast;

extern Cons *make_cons(void *car, void *cdr);
extern Cons *list_reverse(Cons *list);
extern Ast *make_ast_node(AstKind kind, AstVal val);
extern void free_ast_node(Ast *node);

#endif
