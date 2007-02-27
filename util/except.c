#include "except.h"

#include <setjmp.h>
#include <assert.h>
#include "../rha/object.h"

jmp_buf except_stack[EXCEPT_MAX_NESTING];
object_t except_retval[EXCEPT_MAX_NESTING];

int except_tos = -1;

