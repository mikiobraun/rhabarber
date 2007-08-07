#!/usr/bin/env perl

# input file:
#     rha_config.d
#
# output files:
#     rha_init.h
#     rha_init.c
#     rha_types.h


#################################
### step 1: preliminary stuff ###
#################################

$conf_d_fname = "rha_config.d";
$init_h_fname = "rha_init.h";
$init_c_fname = "rha_init.c";
$type_h_fname = "rha_types.h";

$debug = shift @ARGV;

# C identifier
$id = '[a-zA-Z_]\w*';

$keyword = "_rha_";

# load def file into one string
open(FILE, "<$conf_d_fname") or die "Can't open $conf_d_fname: $!";
@input = <FILE>;
close(FILE);
$input = join '', @input;

# parse the def file
$input =~ /\/\/MODULES\s*\n(.*)\/\/TYPES\s*\n(.*)\/\/SYMBOLS\s*\n(.*)/s;
$modules = $1;
$types   = $2;
$symbols = $3;

# (1) disect the includes
@mods = $modules =~ /include\s+\"($id)\.h\"/gox;

# (2) disect the typedefs
@tdefs = $types =~ /$keyword\s+($id)/gox;

# (3) disect the symbols
@symbs = $symbols =~ /($id)\_sym/gox;


############################
# step 2: init all strings #
############################

$type_h_types = $types;
$type_h_ids = "";
$type_h_prototypes = "";
$type_h_symbols = "";

$init_h_modules = $modules;

$init_c_symbols = "";
$init_c_prototypes = "";
$init_c_typeobject = "";
$init_c_functions = "";
$init_c_init_prototypes = "";
$init_c_init_symbols = "";
$init_c_init_typeobjects = "";
$init_c_add_modules = "";


##############################
# step 3: create all strings #
##############################


# first the more complicated stuff: the functions
foreach $module (@mods) {
    # add some stuff
    $init_c_add_modules .= "  ADD_MODULE($module);\n";
    # also add a symbol
    push(@symbs, $module);

    # load the header file
    $module_fname = $module.".h";
    open(FILE, "<$module_fname") or die "Can't open $module_fname: $!";
    @input = <FILE>;
    close(FILE);
    $input = join '', @input;
    # parse it
    while ($input =~ /$keyword\s+($id)\s+($id)\s*\(([^\)]*)\)\s*;/gox) {
	$fntype = $1;
	$fnname = $2;
	$fnargs = $3;
	print "matched: $fntype $fnname $fnargs\n" if $debug;
	
	# disect the args
	@args = $fnargs =~ /($id)[^,]*/gox;
	$narg = $#args + 1;
	# add a symbol for the function
	push(@symbs, $fnname);
	
	# add wrapper code
	$init_c_functions .= "object_t b_$fnname(tuple_t t) {\n";
	$fncall_str = "$fnname(";
	$i = 1;
	foreach $item (@args) {
	    if ($item eq "void") { die "'void' is not allowed as argument" }
	    if ($i>1) { $fncall_str .= ", " }
	    $fnarg_str = "tuple_get(t, $i)";
	    if ($item ne "object_t") { $fnarg_str = "*RAW($item, $fnarg_str)" }
	    $fncall_str .= $fnarg_str;
	    $i++;
	}
	$fncall_str .= ")";
	if ($fntype eq "void") { 
	    $init_c_functions .= "  $fncall_str;\n"
	}
	elsif ($fntype eq "object_t") {
	    $init_c_functions .= "  return $fncall_str;\n" 
	}
	else { 
	    $ucfntype = uc($fntype);
	    $init_c_functions .= "  return wrap($ucfntype, $fncall_str);\n"
	}
	$init_c_functions .= "}\n";

	# add code to add functions
	$init_c_add_modules .= "  add_function(module, $fnname"."_sym";
	$init_c_add_modules .= ", (void *) $fnname, $narg";
	foreach $item (@args) {
	    $ucitem = uc($item);
	    $init_c_add_modules .= ", $ucitem";
	}
	$init_c_add_modules .= ");\n";
    }
}

# after creating a list of symbols, now the easy stuff
create_ids();
create_prototypes();
create_symbols();



##################################
# step 4: create all three files #
##################################
open(FILE, ">$type_h_fname") or die "Can't open $type_h_fname: $!";
print FILE create_type_h();
close FILE;
open(FILE, ">$init_h_fname") or die "Can't open $init_h_fname: $!";
print FILE create_init_h();
close FILE;
open(FILE, ">$init_c_fname") or die "Can't open $init_c_fname: $!";
print FILE create_init_c();
close FILE;

# that's it
exit();


#########################################

sub create_ids {
    $id = 1;
    foreach $item (@tdefs) {
	$ucitem = uc($item);
	$type_h_ids .= "#define $ucitem"."_t $id \n";
	$id++;
    }
}

sub create_prototypes {
    foreach $item (@tdefs) {
	$iitem = substr($item, 0, -2);
	$uciitem = uc($iitem);
	$type_h_prototypes .= "extern object_t $iitem"."_proto;\n";
	$type_h_typeobjects .= "extern object_t $iitem"."_obj;\n";
	$init_c_prototypes .= "object_t $iitem"."_proto;\n";
	$init_c_typeobjects .= "object_t $iitem"."_obj;\n";
	$init_c_init_prototypes .= "  $iitem"."_proto = new();\n";
	$init_c_init_typeobjects .= "  ADD_TYPE($iitem, $uciitem);\n";
    }
}

sub create_symbols {
    # add all @tdefs to @symbs
    foreach $item (@tdefs) {
	push(@symbs, substr($item, 0, -2));
    }

    # get rid of duplicates
    for (@symbs) { $unique{$_} = 1 }
    @unique = keys %unique;
    foreach $item (@unique) {
	$type_h_symbols .= "extern symbol_t $item"."_sym;\n";
	$init_c_symbols .= "symbol_t $item"."_sym;\n";
	$init_c_init_symbols .= "  $item"."_sym = symbol_new(\"$item\");\n";
    }
}

#########################################

sub create_type_h {
    return <<ENDE;
// WARNING: don't edit here!  
// This file has been automatically generated by 'rha_config.pl'.
// Instead edit 'rha_config.d'.

#ifndef DATATYPES_H
#define DATATYPES_H
#define $keyword

// (1) datatypes which are available in Rhabarber
$type_h_types

// (2) primtype id for all types
$type_h_ids

// (3) prototypes for all types
$type_h_prototypes

// (4) type objects
extern object_t void_obj;
$type_h_typeobjects

// (5) symbols
$type_h_symbols

// (6) some useful macros
// get the raw data and convert
//
// for example, raw(int_t, o)
#define RAW(tp, o) ((tp*)(o->raw))
#define ASSERT_RAW_NONZERO(o) assert(raw(o) != 0)

#endif
ENDE
}

#############################

sub create_init_h {
    return <<ENDE;
// WARNING: don't edit here!  
// This file has been automatically generated by 'rha_config.pl'.
// Instead edit 'rha_config.d'.

// (1) includes
$init_h_modules
// (2) 
extern object_t rha_init();
ENDE
}

#############################

sub create_init_c {
    return <<ENDE;
// WARNING: don't edit here!  
// This file has been automatically generated by 'rha_config.pl'.
// Instead edit 'rha_config.d'.

#include <stdarg.h>
#include "alloc.h"
#include "rha_init.h"

// (1) symbols
$init_c_symbols

// (2) prototypes
$init_c_prototypes

// (3) type objects
object_t void_obj;
$init_c_typeobjects

// (4) functions
$init_c_functions

// (5) init
#define ADD_TYPE(ttt, TTT)   // ttt ## _t\
  setptype(ttt ## _proto, TTT ## _T);\
  ttt ## _obj = new();\
  assign(root, ttt ## _sym, ttt ## _obj);\
  assign(ttt ## _obj, proto_sym, ttt ## _proto);

void add_function(object_t module, symbol_t s, int rettype, void *code, int narg, ...)
{
  // create a new object
  object_t o = new_t(FN_T, fn_proto);

  // create a struct containing all info about the builtin function
  fn_t f = ALLOC_SIZE(size_of(fn_t));
  setraw(o, (void *) f);
  f->rettype = rettype;
  f->code = code;
  f->narg = narg;
  f->argtypes = ALLOC_SIZE(narg*size_of(int_t));

  // read out the argument types
  va_list ap;
  va_start(ap, narg);
  for (int i=0; i<narg; i++)
    f->argtypes[i] = va_arg(ap, int_t);
  va_end(ap);

  // finally add it to module
  assign(module, s, o);
}

#define ADD_MODULE(mmm)   // mmm ## .h\
  module = new();\
  assign(modules, mmm ## _sym, module);

object_t rha_init()
{
  object_t root = new();

  // (5.1) create prototypes (TYPES)
$init_c_init_prototypes

  // (5.2) create symbols (SYMBOLS, TYPES, MODULES, functions)
$init_c_init_symbols

  // (5.3) create type objects (TYPES)
$init_c_init_typeobjects

  // (5.4) create the void object
  void_obj = new();
  assign(root, void_sym, void_obj);

  // (5.5) add modules (MODULES, functions)
  object_t modules = new();
  assign(root, modules_sym, modules);
  object_t module = 0;

$init_c_add_modules
}
ENDE
}

#####END#OF#FILE##########
