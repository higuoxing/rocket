#include <assert.h>

#include "tokenizer.h"
#include "vector.h"

int main() {
  {
    Vector *tokens = tokenize("(cons (cons 1 nil))");
    assert(((Token *)vector_get(tokens, 0))->kind == TK_LParen);
    assert(((Token *)vector_get(tokens, 1))->kind == TK_Ident);
    assert(((Token *)vector_get(tokens, 2))->kind == TK_LParen);
    assert(((Token *)vector_get(tokens, 3))->kind == TK_Ident);
    assert(((Token *)vector_get(tokens, 4))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 5))->kind == TK_Ident);
    assert(((Token *)vector_get(tokens, 6))->kind == TK_RParen);
    assert(((Token *)vector_get(tokens, 7))->kind == TK_RParen);
    assert(((Token *)vector_get(tokens, 8))->kind == TK_EOF);
  }

  {
    Vector *tokens = tokenize("(list 1 2 3 4 nil)");
    assert(((Token *)vector_get(tokens, 0))->kind == TK_LParen);
    assert(((Token *)vector_get(tokens, 1))->kind == TK_Ident);
    assert(((Token *)vector_get(tokens, 2))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 2))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 3))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 4))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 5))->kind == TK_Number);
    assert(((Token *)vector_get(tokens, 6))->kind == TK_Ident);
    assert(((Token *)vector_get(tokens, 7))->kind == TK_RParen);
    assert(((Token *)vector_get(tokens, 8))->kind == TK_EOF);
  }
}
