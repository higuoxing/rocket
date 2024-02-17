#ifndef _AST_H_
#define _AST_H_

#include <stdbool.h>

typedef enum AstKind {
  AK_Boolean,
  AK_Number,
  AK_Ident,
  AK_Cons,
} AstKind;

typedef struct Cons {
  void *car;
  void *cdr;
} Cons;

typedef union AstVal {
  /* AK_Boolean */
  bool boolean;
  /* AK_Number */
  double number;
  /* AK_Ident */
  char *ident;
  /* AK_Cons */
  Cons *cons;
} AstVal;

typedef struct AstNode {
  AstKind kind;
  AstVal val;
} Ast;

extern Cons *make_cons(void *car, void *cdr);
extern Cons *list_reverse(Cons *list);
extern Ast *make_ast_node(AstKind kind, AstVal val);

#endif
