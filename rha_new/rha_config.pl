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
	$ucfntype = uc($fntype);
	$init_c_add_modules .= ", $ucfntype, (void *) $fnname, $narg";
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
#define rhabarber

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













###############old stuff

###############################################
### step 2: perform action for each keyword ###
###############################################

# lookfor keywords and accumulate the strings
my $includes = "";
my $builtindefines = "";
my $builtinassigns = "  builtin_tr b;\n";
while ($input =~ /
       (constructor|operator|method|constant)\s+  # keywords
       ($id)\s+       # return type
       ($id)          # function name
       (\s*           # the args part is optional
       \(             # left bracket before args
       ([^\)]*)       # the args, no brackets allowed
       \)\s*)?        # right bracket after args
       ;              # finally, the semicolon
       /gox) {

    my $keyword = $1;
    my $fntype = $2;  # return type
    my $fnname = $3;  # function name
    my $fnargs = $5;  # args, note: $4 is args including brackets
    print "matched: $keyword, $fntype, $fnname, $fnargs\n" if $debug;

    # do we have to include new stuff?
    $includes = include_types($includes, $fntype);

    # catch the 'constant' case
    if ($keyword eq "constant") {
	$builtinassigns .= <<ENDE;
  $class\_tr b_$fnname = $class\_new($fnname);
  object_assign(root, symbol_new(\"$fnname\"), b_$fnname);
ENDE
    }
    else {
	# set default parameters
	my $ellipse = 0;  # false

	# extract types
	my @fnargs = split /,\s*/, $fnargs;  # split by comma
	my $nin = @fnargs+1;
	for ($i=0; $i<@fnargs; $i++) {
	    # assume the first expression is the type
	    $fnargs[$i] =~ /($id|\.\.\.)/o;  
	    $fnargs[$i] = $1;
	    # catch ellipse
	    if ($fnargs[$i] eq "...") {
		# ellipse must be last arg
		if ($i + 1 != @fnargs) {
		    die("Ellipse '...' only allowed as last argument.\n");
		}
		$ellipse = 1; # true must be one, not two or so
	    }
	    else {
		# do we have to include new stuff?
		$includes = include_types($includes, $fnargs[$i]);
	    }
	}
	
	# adjust parameters according to the keyword
	if ($keyword eq "constructor" or $keyword eq "method") {
	    # check fnname has form $class_method
	    if ($fnname !~ /$class\_($id)/) {
		die("$keyword '$fnname' does not have the form '$class\_method'.\n");
	    }
	}
	if ($keyword eq "operator") {
	    # check fnname has form class1_op_class2
	    if ($fnname !~ /$ops/) {
		die("Operator '$fnname' does not have the form 'class1_op_class2' or 'class_op'.\n");
	    }
	}
	if ($keyword eq "method")
	{
	    $nin = @fnargs;
	    check_firstarg($fnargs[0]);
	}
	
	# create code
	$builtindefines .= create_builtintmpl($keyword, $nin, $fntype, $fnname, $ellipse, @fnargs);
	$builtinassigns .= create_builtinassign($keyword, $fnname);
    }
}


##################################################################
### step 3: create the code by substituting into the templates ###
##################################################################

# create $class_stub.h
open(STUBH, ">$class\_stub.h") or die "Can't open $class\_stub.h: $!";
print STUBH create_htmpl($class);
close STUBH;

# create $class_stub.c
open(STUBC, ">$class\_stub.c") or die "Can't open $class\_stub.c: $!";
print STUBC create_ctmpl($class, $includes, $builtindefines, $builtinassigns);
close STUBC;

# that's it
exit;



sub check_firstarg {
    my $arg = shift;
    print "check_firstarg($arg)\n" if $debug;

    my $msg = "First arg of a $class\_method must be";
    if ($c2rha{$arg}) {
	# $arg is one of the types listed in %c2rha
	if ($c2rha{$arg} ne $class) {
	    my $carg = $rha2c{$class};
	    if ($carg) {
		die("$msg $carg or $class\_tr.\n");
	    }
	    else {
		die("$msg $class\_tr.\n");
	    }
	}
	else {
	    return; # ok
	}
    }
    elsif ($arg eq "$class\_tr") {
	return;  # ok
    }
    else {
	die("$msg $class\_tr.\n");
    }
}


sub include_types {
    my $includes = shift;
    my $atype = shift;
    print "include_types(<1 arg omitted>, $atype)\n" if $debug;

    if ($atype eq "object_t" or $atype eq "void") {
	return $includes;
    }
    elsif ($c2rha{$atype}) {
	$rhatype = $c2rha{$atype};
	$includes .= qq/#include "$rhatype\_tr.h"\n/ if $includes !~ /$rhatype\_tr/;
	return $includes;
    }
    elsif ($atype =~ /$id\_tr/) {
	$includes .= qq/#include "$atype.h"\n/ if $includes !~ /$atype/;
	return $includes;
    }
    else {
	die("Unknown type :'$atype'.\n");
    }
}


sub create_builtinassign {
    my $keyword = shift;
    my $fnname = shift;
    print "create_builtinassign($keyword, $fnname)\n" if $debug;

    if ($fnname =~ /($id)_($ops)_($id)/) {
	my $class1 = $1;
	my $op = $2;
	my $class2 = $3;
	return "  b = builtin_new(b_$fnname);\n"
             . "  object_assign(bi, symbol_new(\"$fnname\"), b);\n"
             . "  overloaded_add(object_lookup(root, op_$op\_sym), b, tuple_make2(primtype_obj($class1\_type), primtype_obj($class2\_type)));\n";
    }
    elsif ($fnname =~ /($id)_($ops)/) {
	my $class = $1;
	my $op = $2;
	return "  b = builtin_new(b_$fnname);\n"
             . "  object_assign(bi, symbol_new(\"$fnname\"), b);\n"
             . "  overloaded_add(object_lookup(root, op_$op\_sym), b, tuple_make1(primtype_obj($class\_type)));\n";
    }
    elsif ($fnname =~ /($class)_from_($id)/) {
	my $class1 = $1;
	my $class2 = $2;
	if ($class1 ne $class) { die("Converter $fnname must begin with '$class'.\n"); }
	if ($keyword ne "constructor") { die("Converter $fnname should be declared as 'constructor'.\n"); }
	return <<ENDE;
  {
    overloaded_tr ov = object_lookup(primtype_obj($class\_type), from\_sym);
    if (!ov) { 
	ov = overloaded_new(); 
	object_assign(primtype_obj($class\_type), from_sym, ov); 
	object_assign(ov, symbol_new("name"), string_new("from")); 
    }
    overloaded_add(ov, builtin_new(b_$fnname), tuple_make1(primtype_obj($class2\_type)));
  }
ENDE
    }
    else {
	# not an operator, just a method
	$fnname =~ /$class\_($id)/;
	my $method = $1;
	return "  b = builtin_new(b_$fnname);\n"
             . "  object_assign(bi, symbol_new(\"$fnname\"), b);\n"
             . "  object_assign(pobj, symbol_new(\"$method\"), builtin_new(&b_$fnname));\n";
    }
}


sub create_builtintmpl {
    my $keyword = shift;
    my $nin = shift;
    my $fntype = shift;
    my $fnname = shift;
    my $ellipse = shift;
    my @fnargs = @_;
    print "create_builtintmpl($keyword, $nin, $fntype, $fnname, $ellipse, @fnargs)\n" if $debug;

    my $getchecks;
    my $callargs;
    if (@fnargs > 0) {
	for ($i=0; $i<(@fnargs-$ellipse); $i++) {
	    $getchecks .= create_tupleget($keyword, $i);
	    $getchecks .= create_typecheck($keyword, $fnname, $fnargs[$i], $i);
	    $callargs .= ", " if ($i > 0);
	    $callargs .= create_callargs($keyword, $fnargs[$i], $i);
	}
    }
    my $fncall;
    if ($ellipse) {
	$fncall = create_fncall_ellipse($fntype, $fnname, $callargs, $nin);
    }
    else {
	$fncall = create_fncall($fntype, $fnname, $callargs);
    }

    # finally create the template (with special case for ellipse)
    my $nargcheck;
    if ($ellipse) {
	$nargcheck = "if (tuple_length(in) >= $nin) {\n";
	$nargerror = "rha_error(\"$fnname: at least $nin arguments expected, got %d.\\n\", tuple_length(in));";
    }
    else {
	$nargcheck = "if (tuple_length(in) == $nin) {\n";
	$nargerror = "rha_error(\"$fnname: exactly $nin arguments expected, got %d.\\n\", tuple_length(in));";
    }
    return <<ENDE;
object_t b_$fnname(object_t env, tuple_tr in)
{
  $nargcheck$getchecks$fncall
    return res;
  }
  else {
    $nargerror
  }
  return 0;
}


ENDE
}


sub create_tupleget {
    my $keyword = shift;
    my $i = shift;
    print "create_tupleget($keyword, $i)\n" if $debug;

    if ($keyword =~ /constructor|operator/) {
	$i = $i + 1;
    }
    if ($keyword eq "method" and $i==0) {
	return "    assert(env != 0);\n"
	     . "    object_t t$i = env;\n";
    }
    else {
	return "    object_t t$i = tuple_get(in, $i);\n";
    }
}


sub create_typecheck {
    my $keyword = shift;
    my $fnname = shift;
    my $fnarg = shift;
    my $i = shift;
    print "create_typecheck($keyword, $fnname, $fnarg, $i)\n" if $debug;

    if ($keyword =~ /constructor|operator/) {
	$i = $i + 1;
    }
    if ($c2rha{$fnarg}) {
	$fnarg = "$c2rha{$fnarg}_tr";
    }
    
    if ($fnarg eq "object_t" or $fnarg eq "plain_tr") {
	# no check in this case
	return "";
    }
    elsif ($fnarg =~ /($id)\_tr/) {
	return <<ENDE;
    if (!HAS_TYPE($1, t$i)) {
	rha_error("$fnname: $i$ordinal[$i] arg has wrong type %s, "
		  "should be $1.\\n", primtype_name(object_primtype(t$i)));
	return 0;
    }
ENDE
    }
    else {
	die("'$fnarg' must be $ctypes, object_t something like class_tr.\n");
    }
}


sub create_callargs {
    my $keyword = shift;
    my $fnarg = shift;
    my $i = shift;
    print "create_callargs($keyword, $fnarg, $i)\n" if $debug;
    
    if ($keyword =~ /constructor|operator/) {
	$i = $i + 1;
    }
    if ($c2rha{$fnarg}) {
	return "$c2rha{$fnarg}_get(t$i)";
    }
    elsif ($fnarg eq "object_t" or $fnarg =~ /($id)\_tr?/) {
        return "t$i";
    }
    else {
	die("'$fnarg' must be $ctypes, object_t or something like class_tr.\n");
    }
}


sub create_fncall_ellipse {
    my $fntype = shift;
    my $fnname = shift;
    my $callargs = shift;
    my $nin = shift;
    print "create_fncall_ellipse($fntype, $fnname, $callargs, $nin)\n" if $debug;

    if ($fntype eq "void") {
	return <<ENDE;
    switch (tuple_length(in)) {
      case $nin:   $fnname($callargs); break;
      case $nin+1: $fnname($callargs, tuple_get(in, $nin)); break;
      case $nin+2: $fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1)); break;
      case $nin+3: $fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1), tuple_get(in, $nin+2)); break;
      default: rha_error("Too many args for ellipse\\n.");
    }
    object_t res = 0;
ENDE
    }
    elsif ($c2rha{$fntype}) {
	return <<ENDE;
    object_t res;
    switch (tuple_length(in)) {
      case $nin:   res = $c2rha{$fntype}_new($fnname($callargs)); break;
      case $nin+1: res = $c2rha{$fntype}_new($fnname($callargs, tuple_get(in, $nin)); break;
      case $nin+2: res = $c2rha{$fntype}_new($fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1)); break;
      case $nin+3: res = $c2rha{$fntype}_new($fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1),tuple_get(in, $nin+2)); break;
      default: rha_error("Only up to three args allowed for ellipse '...'.\\n.");
    }
ENDE
    }
    elsif ($fntype eq "object_t" or $fntype =~ /($id)\_tr/) {
	return <<ENDE;
    object_t res;
    switch (tuple_length(in)) {
      case $nin:   res = $fnname($callargs); break;
      case $nin+1: res = $fnname($callargs, tuple_get(in, $nin)); break;
      case $nin+2: res = $fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1)); break;
      case $nin+3: res = $fnname($callargs, tuple_get(in, $nin), tuple_get(in, $nin+1), tuple_get(in, $nin+2)); break;
      default: rha_error("Only up to three args allowed for ellipse '...'.\\n.");
    }
ENDE
    }
    else {
	die("Return type must be $ctypes, object_t or something like class_tr.\n");
    }
}


sub create_fncall {
    my $fntype = shift;
    my $fnname = shift;
    my $callargs = shift;
    print "create_fncall($fntype, $fnname, $callargs)\n" if $debug;

    if ($fntype eq "void") {
	return "    $fnname($callargs);\n    object_t res = 0;";
    }
    elsif ($c2rha{$fntype}) {
	return "    object_t res = $c2rha{$fntype}_new($fnname($callargs));";
    }
    elsif ($fntype eq "object_t" or $fntype =~ /($id)\_tr/) {
	return "    object_t res = $fnname($callargs);";
    }
    else {
	die("Return type must be $ctypes, object_t or something like class_tr.\n");
    }
}


sub create_htmpl {
    my $class = shift;
    print "create_htmpl($class)\n" if $debug;

    my $CLASS = uc $class;

    return <<ENDE;
/*
 * This file has been automatically generated by rha_sub.pl
 * DO NOT EDIT!
 *
 * rha_stub.pl written by Stefan Harmeling
 *   with modifications by Mikio Braun
 */

#ifndef $CLASS\_STUB_H
#define $CLASS\_STUB_H

#include "object.h"

extern void $class\_stub_init(object_t root);

#endif
ENDE
}


sub create_ctmpl {
    my $class = shift;
    my $includes = shift;
    my $builtindefines = shift;
    my $builtinassigns = shift;
    print "create_ctmpl($class, <3 args omitted>)\n" if $debug;
    
    return <<ENDE;
/*
 * This file has been automatically generated by rha_sub.pl
 * DO NOT EDIT!
 *
 * rha_stub.pl written by Stefan Harmeling
 *   with modifications by Mikio Braun
 */

#include "$class\_stub.h"

#include "$class\_tr.h"
#include <stdio.h>
#include "debug.h"
#include "core.h"
#include "messages.h"
#include "none_tr.h"
#include "string_tr.h"
#include "tuple_tr.h"
#include "overloaded_tr.h"
#include "plain_tr.h"
#include "builtin_tr.h"
$includes

$builtindefines

void $class\_stub_init(object_t root)
{
  printdebug("executing $class\_stub_init(root)");

  // requires
  symbol_init();
  builtin_init();
  $class\_init();
  plain_init();

  object_t pobj = primtype_obj($class\_type);
  object_assign(root, symbol_new("$class"), pobj);
  object_assign(pobj, symbol_new("type"), pobj);
  object_assign(pobj, symbol_new("typename"), string_new("$class"));
  object_assign(pobj, symbol_new("$class"), pobj);
  object_t bi = plain_new();
  object_setparent(bi, 0);
  object_assign(pobj, symbol_new("builtins"), bi);

$builtinassigns}
ENDE
}



