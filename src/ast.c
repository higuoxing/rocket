#include "common.h"

#include "ast.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#if 0
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
#endif

AstNode *make_ast_bool(bool b) {
  AstBool *ast = (AstBool *)malloc(sizeof(AstBool));
  ast->base.kind = AST_BOOL;
  ast->boolean = b;
  return (AstNode *)ast;
}

AstNode *make_ast_char(char c) {
  AstChar *ast = (AstChar *)malloc(sizeof(AstChar));
  ast->base.kind = AST_CHAR;
  ast->char_ = c;
  return (AstNode *)ast;
}

AstNode *make_ast_number(double d) {
  AstNumber *ast = (AstNumber *)malloc(sizeof(AstNumber));
  ast->base.kind = AST_NUMBER;
  ast->number = d;
  return (AstNode *)ast;
}

AstNode *make_ast_ident(const char *id) {
  AstIdent *ast = (AstIdent *)malloc(sizeof(AstIdent));
  ast->base.kind = AST_IDENT;
  ast->ident = strdup(id);
  return (AstNode *)ast;
}

AstNode *make_ast_proc_call(AstNode *callable, Vector *args) {
  AstProcCall *ast = (AstProcCall *)malloc(sizeof(AstProcCall));
  ast->base.kind = AST_PROC_CALL;
  ast->callable = callable;
  ast->args = args;
  return (AstNode *)ast;
}

void free_ast_node(AstNode *node) {
  switch (node->kind) {
  case AST_BOOL:
  case AST_CHAR:
  case AST_NUMBER:
    free(node);
    break;
  case AST_IDENT: {
    AstIdent *ast = (AstIdent *)node;
    free(ast->ident);
    free(ast);
    break;
  }
  case AST_PROC_CALL: {
    AstProcCall *ast = (AstProcCall *)node;
    free_ast_node(ast->callable);
    for (int i = 0; i < vector_len(ast->args); ++i)
      free_ast_node((AstNode *)vector_get(ast->args, i));
    free_vector(ast->args);
    free(ast);
    break;
  }
  default:
    fprintf(stderr, "%s: unrecognized node type", __FUNCTION__);
    exit(1);
  }
}
