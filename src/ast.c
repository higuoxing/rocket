#include "common.h"

#include "ast.h"

Cons *make_cons(void *car, void *cdr) {
  Cons *cons = (Cons *)malloc(sizeof(Cons));
  cons->car = car;
  cons->cdr = cdr;
  return cons;
}

Cons *list_reverse(Cons *list) {
  Cons *tail = NULL;
  if (!list)
    return NULL;

  while (list) {
    Cons *curr = list;
    tail = make_cons(curr->car, tail);
    list = curr->cdr;
    free(curr);
  }

  return tail;
}

Ast *make_ast_node(AstKind kind, AstVal val) {
  Ast *node = (Ast *)malloc(sizeof(Ast));
  node->kind = kind;
  node->val = val;
  return node;
}
