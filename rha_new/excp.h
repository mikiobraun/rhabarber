#ifndef EXCP_FN_H
#define EXCP_FN_H

#include "rha_init.h"

extern       void     excp_init(any_t, any_t);
extern _rha_ any_t excp_new(string_t msg);
extern _rha_ string_t excp_msg(any_t excp);
extern _rha_ void     excp_show(any_t excp);
extern       any_t excp_newf(const char *fmt, ...);
#endif
