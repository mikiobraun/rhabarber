#ifndef EVAL_H
#define EVAL_H

#include <stdlib.h>
#include <setjmp.h>
#include "object.h"

extern       void     eval_init(object_t root, object_t module);
extern _rha_ object_t eval(object_t, object_t);
extern _rha_ object_t callslot(object_t obj, symbol_t slotname, int_t narg, ...);
extern       object_t vcallslot(object_t obj, symbol_t slotname, int_t narg, list_t);

// used by parse.c
// using call_fun instead of "wraping, eval, unwrapping" saves some time
extern object_t call_fun(object_t env, int_t tlen, tuple_t expr);

//////////////////
extern object_t eval_currentlocation;

// some cool macros for 'try' and 'catch' stuff
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
      rha_error("maximal frame nesting reached");		\
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

#include "excp.h"

#define frame_jump(_type, _expr) do {					\
    if ((_type) == TRY_FRAME)						\
      while ( (frame_tos >= 0)						\
              && ((_type) != frame_type[frame_tos]) )			\
	frame_tos--;							\
    while ( (frame_tos >= 0)						\
            && ((_type) < frame_type[frame_tos]) )			\
      frame_tos--;							\
    if ((frame_tos < 0) || ((_type) != frame_type[frame_tos])) {	\
      frame_tos = 0;							\
      if ((_type == FUNCTION_FRAME))					\
        rha_error("no function to 'return' from");			\
      else if ((_type == LOOP_FRAME))					\
        rha_error("no loop to 'break'");				\
      else if ((_type == BLOCK_FRAME))					\
        rha_error("no block to 'return' from");				\
      else if ((_type == TRY_FRAME)) {					\
	excp_show(_expr);						\
        fprintf(stderr,							\
		"[fatal error] rhabarber failed to catch last exception\n"); \
        exit(EXIT_FAILURE);						\
      }									\
    }									\
    else {								\
      frame_value[frame_tos] = (_expr);					\
      longjmp(frame_stack[frame_tos], 1);				\
    }									\
  } while (0);


// WARNING: don't call 'return' inside 'try-catch' blocks!!!
#define try begin_frame(TRY_FRAME)

#define catch(_exception) while ((--frame_tos)-frame_tos);      \
    else if (((_exception) = frame_value[frame_tos--]) && 0);   \
    else

#define throw(_expr) frame_jump(TRY_FRAME, (_expr))

#endif
