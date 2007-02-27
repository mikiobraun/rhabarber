#ifndef LEXER_TR_H
#define LEXER_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct lexer_s *lexer_tr;
extern primtype_t lexer_type;
extern void lexer_init(void);

extern constructor lexer_tr lexer_new(lexer value);

extern method void          lexer_add(lexer_tr p, list_tr l);
extern method list_t        lexer_lex(lexer_tr p, string_tr s);
extern method string_t      lexer_to_string(lexer p);

#endif
