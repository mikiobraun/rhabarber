#ifndef EXCP_FN_H
#define EXCP_FN_H

#include "rha_init.h"

extern       void     excp_init(object_t, object_t);
extern _rha_ object_t excp_new(string_t msg);
extern _rha_ void     excp_show(object_t excp);
extern       object_t excp_newf(const char *fmt, ...);
#endif
