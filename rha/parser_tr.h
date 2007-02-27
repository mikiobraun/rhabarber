#ifndef PARSER_TR_H
#define PARSER_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"
#include "bool_tr.h"
#include "real_tr.h"
#include "string_tr.h"
#include "list_tr.h"

typedef struct parser_s *parser_tr;
extern primtype_t parser_type;
extern void parser_init(void);

extern constructor parser_tr parser_new();
extern method void parser_verbose(parser_tr p);
extern method void parser_addrule(parser_tr p, object_t fun, string_tr lhs, string_tr rhs, int_tr prec, bool_tr left);
extern method void parser_changerule(parser_tr p, int i, object_t fun, string_tr lhs, string_tr rhs, int_tr prec, bool_tr left);
extern method void parser_removerule(parser_tr p, int i);
extern method void parser_keywords(parser_tr p);
extern method void parser_update(parser_tr p);
extern method void parser_table(parser_tr p);
extern method void parser_list(parser_tr p);
extern method object_t parser_parse(parser_tr p, string_tr s);
extern method string_t parser_to_string(parser_tr p);

#endif
