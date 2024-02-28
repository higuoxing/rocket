#include "common.h"

#include "tokenizer.h"
#include "vector.h"

int main() {
  {
    Vector *tokens = tokenize("  ", "");
    assert(((Token *)vector_get(tokens, 0))->kind == TOKEN_EOF);
    for (int i = 0; i < tokens->len; ++i)
      free_token((Token *)tokens->items[i]);
    free_vector(tokens);
  }

  {
    Vector *tokens = tokenize("(cons (cons 1 nil))", "");
    assert(((Token *)vector_get(tokens, 0))->kind == TOKEN_LPAREN);
    assert(((Token *)vector_get(tokens, 1))->kind == TOKEN_IDENT);
    assert(((Token *)vector_get(tokens, 2))->kind == TOKEN_LPAREN);
    assert(((Token *)vector_get(tokens, 3))->kind == TOKEN_IDENT);
    assert(((Token *)vector_get(tokens, 4))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 5))->kind == TOKEN_IDENT);
    assert(((Token *)vector_get(tokens, 6))->kind == TOKEN_RPAREN);
    assert(((Token *)vector_get(tokens, 7))->kind == TOKEN_RPAREN);
    assert(((Token *)vector_get(tokens, 8))->kind == TOKEN_EOF);
    for (int i = 0; i < tokens->len; ++i)
      free_token((Token *)tokens->items[i]);
    free_vector(tokens);
  }

  {
    Vector *tokens = tokenize("(list 1 2 3 4 nil)", "");
    assert(((Token *)vector_get(tokens, 0))->kind == TOKEN_LPAREN);
    assert(((Token *)vector_get(tokens, 1))->kind == TOKEN_IDENT);
    assert(((Token *)vector_get(tokens, 2))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 2))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 3))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 4))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 5))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 6))->kind == TOKEN_IDENT);
    assert(((Token *)vector_get(tokens, 7))->kind == TOKEN_RPAREN);
    assert(((Token *)vector_get(tokens, 8))->kind == TOKEN_EOF);
    for (int i = 0; i < tokens->len; ++i)
      free_token((Token *)tokens->items[i]);
    free_vector(tokens);
  }

  {
    Vector *tokens = tokenize("(+ 1 2 3 4 5)", "");
    assert(((Token *)vector_get(tokens, 0))->kind == TOKEN_LPAREN);
    assert(((Token *)vector_get(tokens, 1))->kind == TOKEN_IDENT);
    assert(strcmp(((Token *)vector_get(tokens, 1))->literal, "+") == 0);
    assert(((Token *)vector_get(tokens, 2))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 3))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 4))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 5))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 6))->kind == TOKEN_NUMBER);
    assert(((Token *)vector_get(tokens, 7))->kind == TOKEN_RPAREN);
    assert(((Token *)vector_get(tokens, 8))->kind == TOKEN_EOF);
    for (int i = 0; i < tokens->len; ++i)
      free_token((Token *)tokens->items[i]);
    free_vector(tokens);
  }
}
