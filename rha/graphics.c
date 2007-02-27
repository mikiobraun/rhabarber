#include "graphics.h"

// for Stefan's laptop there is the SIMPLE choice which switches opengl
// stuff on and off

#ifndef SIMPLE
#include <GL/gl.h> 
#include <GL/glut.h> 
#endif

#include <assert.h>
#include <stdio.h>
#include "builtin_tr.h"
#include "tuple_tr.h"
#include "object.h"
#include "matrix_tr.h"
#include "string_tr.h"
#include "eval.h"
#include "symbol_tr.h"
#include "core.h"

static symbol_tr gr_style_sym = 0;  // "style"
static symbol_tr gr_dash_sym = 0;   // "-"
static symbol_tr gr_dot_sym = 0;    // "."

static list_tr graphics_buffer = 0;

void graphics_init_symbols(void) 
{
  gr_style_sym = symbol_new("style");
  gr_dash_sym = symbol_new("-");
  gr_dot_sym = symbol_new(".");
}

void graphics_clear(void);

BUILTIN(watch);
BUILTIN(unwatch);

void graphics_init(int argc, char *argv[])
{
  // require
  list_init();

  /*
  // start the opengl, glut stuff
  glutInit(&argc, argv); 
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB); 
  glutInitWindowSize (250, 250); 
  glutInitWindowPosition (100, 100); 
  glutCreateWindow ("rhabarber"); 

  // select clearing (background)
  glClearColor (1.0, 1.0, 1.0, 1.0); 

  // initialize viewing values 
  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity(); 
  glOrtho(-3.0, 3.0, -1.0, 2.0, -1.0, 1.0); 

  glutHideWindow();
  */
  // initalize object list
  graphics_clear();
}

void graphics_stub_init(object_t root) 
{
  // requires
  symbol_init();
  builtin_init();

  // initialize symbols for graphics toolbox
  graphics_init_symbols();

  // add builtin function defined here
  ADDBUILTIN(root, "watch", watch);
  ADDBUILTIN(root, "unwatch", unwatch);
}

object_t identity_fn(object_t obj)
{
  // fn will be (fn obj)
  tuple_tr fn = tuple_new(2);
  tuple_set(fn, 0, fn_sym);
  tuple_set(fn, 1, obj);

  // fncall will be ((fn obj))
  tuple_tr fncall = tuple_new(1);
  tuple_set(fncall, 0, fn);
  return fncall;
}

object_t graphics_assign(object_t this, object_t env, tuple_tr in)
{  
  // determine style
  symbol_tr style = gr_dash_sym;   // default
  if (tuple_length(in) == 4) {
    object_t in3 = tuple_get(in, 3);
    CHECK_TYPE(string, in3);
    style = symbol_new(string_get(in3));
  }

  // create a special graphics object -> here should be more stuff
  object_t obj = tuple_new(4);

  // assign data
  object_assign(obj, gr_style_sym, style);
  object_t in1 = tuple_get(in, 1);
  object_t in2 = tuple_get(in, 2);
  tuple_set(obj, 0, identity_fn(in1));
  tuple_set(obj, 1, identity_fn(in2));
  tuple_set(obj, 2, this);
  tuple_set(obj, 3, env);
  list_append(graphics_buffer, obj);
  return obj;
}

void graphics_clear(void)
{
  // remove all objects
  graphics_buffer = list_new();
  // update display
  graphics_display();
}


void object_draw(object_t obj) {
  if (HAS_TYPE(tuple, obj)) {
    assert(tuple_length(obj)==4);
    object_t this = tuple_get(obj, 2);
    object_t env;
    if (this) env = cons_new(this, tuple_get(obj, 3));
    else env = tuple_get(obj, 3);
    matrix_tr x = eval(env, tuple_get(obj, 0));
    matrix_tr y = eval(env, tuple_get(obj, 1));
    if (x && y && HAS_TYPE(matrix, x) && HAS_TYPE(matrix, y)) {
      int lenx = matrix_length(x);
      int leny = matrix_length(y);
      if (lenx == leny) {
	object_t style = rha_lookup(obj, gr_style_sym);
	if (style) CHECK_TYPE(symbol, style);
	else style = gr_dash_sym;
#ifndef SIMPLE
	if (symbol_equal_symbol(style, gr_dash_sym)) {
	  glBegin(GL_LINE_STRIP);
	}
	else if (symbol_equal_symbol(style, gr_dot_sym)) {
	  glPointSize(3.0);
	  glBegin(GL_POINTS);
	}
	for (int i = 0; i < lenx; i++) {
	  glVertex2f(matrix_getl(x, i), matrix_getl(y, i));
	}
	glEnd();
#endif
	//	maxx = matrix_max(x);
	//	minx = matrix_min(x);
	//	maxy = matrix_max(y);
	//	miny = matrix_min(y);
        return;
      }
    }
  }
  printf("Don't know how to watch the passed objects.\n");
}

void graphics_display(void)
{
  /*
  // clear all pixels
  glClear (GL_COLOR_BUFFER_BIT); 

  // drawing color blue
  glColor3f (0.0, 0.0, 1.0); 

  // loop over the object in the graphics_buffer
  object_t x; // iter var
  list_foreach(x, graphics_buffer)
    object_draw(x);

  // show what is in the opengl buffer
  glFlush (); 
  */
} 




/*
 *   function that can be called from inside rhabarber
 */ 

BUILTIN(watch)
{
/*   // for now create random matrices */
/*   tuple_set(in, 1, matrix_rand(5,1)); */
/*   tuple_set(in, 2, matrix_rand(5,1)); */

  // check inputs
  int nin = tuple_length(in);
  if (nin!=4 && nin!=3) {
    printf("\"watch\" requires two or three args.\n");
    return 0;
  }
  if (nin==4) {
    object_t in3 = tuple_get(in, 3);
    if (!HAS_TYPE(string, in3)) {
      printf("Third arg of \"watch\" must be a string.\n");
      return 0;
    }
  }
#ifndef SIMPLE
  glutShowWindow();
#endif
  return graphics_assign(this, env, in);
}


BUILTIN(unwatch)
{
  if (tuple_length(in)==1) {
    graphics_clear();
#ifndef SIMPLE
    glutHideWindow();
#endif
    return 0;
  }
  printf("\"unwatch\" should be called w/o args, i.e. \"unwatch()\".\n");
  return 0;
}
