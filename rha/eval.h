#ifndef EVAL_H
#define EVAL_H

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "list_tr.h"

extern object_t resolve_eval_list(object_t env, list_tr exprlist);
extern object_t eval(object_t env, object_t expr);
extern object_t call(object_t this, object_t env, tuple_tr t);
extern object_t assigntosymbol(object_t env, object_t lhs, object_t rhs);
bool iscallof(symbol_tr s, object_t t);

extern object_t eval_fctcall_hook(object_t env, object_t tuple);

extern object_t eval_currentlocation;

#define FRAME_MAX_NESTING 128
extern jmp_buf frame_stack[FRAME_MAX_NESTING];  // the stacked frames
extern object_t frame_value[FRAME_MAX_NESTING]; // the value to return
extern int frame_type[FRAME_MAX_NESTING];       // the type of the frame
#define FUNCTION_FRAME 0
#define LOOP_FRAME     1
#define BLOCK_FRAME    2
#define TRY_FRAME      3
extern int frame_tos;                           // frame counter
// Note, that the frames would be better stored in a dynamic data
// structure, because recursion can easily reach the maximum nesting.

#define begin_frame(_type)                                      \
    if (frame_tos >= FRAME_MAX_NESTING - 1) {                   \
      frame_tos = -1;                                           \
      rha_error("Maximal frame nesting reached.\n");            \
    }                                                           \
    else if ( (frame_type[++frame_tos] = (_type)) - (_type) );  \
    else if ( !setjmp(frame_stack[frame_tos]) ) do

#define end_frame(_result) while ((--frame_tos)-frame_tos);     \
    else {                                                      \
      _result = frame_value[frame_tos--];                       \
    }


// note the 'frame_tos = 0;'
// this is assuming that there is some outer try-catch-block in the
// read-eval-loop in rhabarber.c
// without 'frame_tos = 0;' we would miss an exception that is
// generated in frame_jump by rha_error if we for instance just type
// 'return' at the prompt.
#define frame_jump(_type, _expr) do {                           \
    if ((_type) == TRY_FRAME)                                   \
      while ( (frame_tos >= 0)                                  \
              && ((_type) != frame_type[frame_tos]) )           \
                frame_tos--;                                    \
    while ( (frame_tos >= 0)                                    \
            && ((_type) < frame_type[frame_tos]) )              \
	      frame_tos--;                                      \
    if ((frame_tos < 0) || ((_type) != frame_type[frame_tos])) {\
      frame_tos = 0;                                            \
      if ((_type == FUNCTION_FRAME))                            \
        rha_error("No function to 'return' from.\n");           \
      else if ((_type == LOOP_FRAME))                           \
        rha_error("No loop to 'break'.\n");                     \
      else if ((_type == BLOCK_FRAME))                          \
        rha_error("No block to 'return' from.\n");              \
      else if ((_type == TRY_FRAME)) {                          \
        fprintf(stderr, "Rhabarber failed to catch exception.\n"); \
        exit(EXIT_FAILURE);                                     \
      }                                                         \
    }                                                           \
    else {                                                      \
      frame_value[frame_tos] = (_expr);                         \
      longjmp(frame_stack[frame_tos], 1);                       \
    }                                                           \
  } while (0);

#define try begin_frame(TRY_FRAME)

#define catch(_exception) while ((--frame_tos)-frame_tos);      \
    else if (((_exception) = frame_value[frame_tos--]) && 0);   \
    else

#define throw(_expr) frame_jump(TRY_FRAME, (_expr))

#endif
