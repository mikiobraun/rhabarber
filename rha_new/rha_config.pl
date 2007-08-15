#!/usr/bin/env perl

# input file:
#     rha_config.d
#
# output files:
#     rha_init.h
#     rha_init.c
#     rha_types.h

# TODO:
# *   automatically ensure the right order of modules
# *   remove the module name from the beginning of function names (MAYBE)


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
$input =~ /\/\/MODULES\s*\n(.*)\/\/TYPES\s*\n(.*)\/\/SYMBOLS\s*\n(.*)\/\/MACROS\s*\n(.*)\/\/PRULES\s*\n(.*)/s;
$modules = $1;
$types   = $2;
$symbols = $3;
$macros  = $4;
$prules  = $5;

# (0) remove comments
$modules =~ s/\/\/[^\n]*//g;
$types =~ s/\/\/[^\n]*//g;
$symbols =~ s/\/\/[^\n]*//g;
$macros =~ s/\/\/[^\n]*//g;
$prules =~ s/\/\/[^\n]*//g;

# (1) disect the includes
@mods = $modules =~ /include\s+\"($id)\.h\"/gox;

# (2) disect the typedefs
@tdefs = $types =~ /$keyword\s+($id)/gox;

# (3) disect the symbols
@symbs = $symbols =~ /($id)\_sym/gox;

# (4) create a hash of types
%typeset = map { $_ => 1 } @tdefs;


############################
# step 2: init all strings #
############################

$type_h_types = $types;
$type_h_ids = "";
$type_h_prototypes = "";
$type_h_symbols = "";
$type_h_macros = $macros;

$init_h_modules = $modules;

$init_c_symbols = "";
$init_c_prototypes = "";
$init_c_typeobject = "";
$init_c_ptypenames = "";
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
    $input =~ s/\/\/[^\n]*//g;  # remove comments

    # parse it
    while ($input =~ /$keyword\s+($id)\s+($id)\s*\(([^\)]*)\)\s*;/gox) {
	$fntype = $1;
	$fnname = $2;
	$fnargs = $3;
	print "matched: $fntype $fnname $fnargs\n" if $debug;
	
	# disect the args
	@args = $fnargs =~ /($id)[^,]*/gox;
	$narg = $#args + 1;
	
        # find C elipses '...'
	$elispes = false;
	if ($fnargs =~ /\.\.\.\s*/gox) {
	    $elipses = true;
	}

	# check whether all types appear in '%typeset'
	if (!$typeset{$fntype} && ($fntype ne "void")) {
	    die "type error: return type '$fntype' of '$fnname' in '$module_fname' not in 'rha_config.d'\n";
	}
	foreach $arg (@args) {
	    if (!$typeset{$arg}) {
		die "type error: arg type '$arg' of '$fnname' in '$module_fname' not in 'rha_config.d'\n";
	    }
	}
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
	    $ucitem = uc($item);
	    if ($item eq "int_t") { $fnarg_str = "UNWRAP_INT($fnarg_str)" }
	    elsif ($item eq "bool_t") { $fnarg_str = "UNWRAP_BOOL($fnarg_str)" }
	    elsif ($item eq "symbol_t") { $fnarg_str = "UNWRAP_SYMBOL($fnarg_str)" }
	    elsif ($item eq "real_t") { $fnarg_str = "UNWRAP_REAL($fnarg_str)" }
	    elsif ($item ne "object_t") { $fnarg_str = "UNWRAP_PTR($ucitem, $fnarg_str)" }
	    $fncall_str .= $fnarg_str;
	    $i++;
	}
	$fncall_str .= ")";
	if ($fntype eq "int_t") { $fncall_str = "WRAP_INT($fncall_str)" }
	elsif ($fntype eq "bool_t") { $fncall_str = "WRAP_BOOL($fncall_str)" }
	elsif ($fntype eq "symbol_t") { $fncall_str = "WRAP_SYMBOL($fncall_str)" }
	elsif ($fntype eq "real_t") { $fncall_str = "WRAP_REAL($fncall_str)" }
	elsif ($fntype ne "object_t" && $fntype ne "void") {
	    $ucfntype = uc($fntype);
	    $ifntype = 	substr($fntype, 0, -2);
	    $fncall_str = "WRAP_PTR($ucfntype, $ifntype"."_proto, $fncall_str)";
	}
	if ($fntype eq "void") { $init_c_functions .= "  $fncall_str;\n  return void_obj;\n" }
	else { $init_c_functions .= "  return $fncall_str;\n" }
	$init_c_functions .= "}\n";

	# add code to add functions
	$init_c_add_modules .= "  add_function(module, $fnname"."_sym";
	$init_c_add_modules .= ", b_$fnname, $narg";
	foreach $item (@args) {
	    $ucitem = uc($item);
	    $init_c_add_modules .= ", $ucitem";
	}
	$init_c_add_modules .= ");\n";
    }

    # check for $module_init function
    if ($input =~ /$module\_init/) {
	if ($input =~ /void\s+$module\_init\(object_t\s*(\s$id\s*)?,\s*object_t\s*(\s$id\s*)?\)/) {
	    print $module, "_init(object_t, object_t) found\n" if $debug;
	    $init_c_add_modules .= "  $module"."_init(root, module);\n";
	}
	else {
	    die "type error, correct signature: 'void $module"."_init(object_t, object_t)'";
	}
    }
    $init_c_add_modules .= "\n";
}
chop($init_c_add_modules);
chop($init_c_add_modules);

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
    $start = 0;
    $id = $start;
    foreach $item (@tdefs) {
	if ($id>$start) { 
	    $type_h_ids .= ",\n";
	    $init_c_ptypenames .= ",\n";
	}
	$ucitem = uc($item);
	$iucitem = substr($ucitem, 0, -2);
	$type_h_ids .= "  $ucitem = $id";
	$init_c_ptypenames .= "    \"$iucitem\"";
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

// (0) use the keyword to mark the datatypes and function in
// 'rha_config.d' that should be accessible in Rhabarber
#define $keyword

// (1) datatypes which are available in Rhabarber
$type_h_types
// (2) primtype id for all types
enum ptypes {
$type_h_ids
};


// (3) prototypes for all types
$type_h_prototypes

// (4) type objects
extern object_t void_obj;
$type_h_typeobjects

// (5) ptype names
extern string_t ptype_names[];

// (6) symbols
$type_h_symbols

// (7) macros
$type_h_macros
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
// (2) 'rha_init' creates the whole object hierarchy
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
#include <string.h>
#include <stdio.h>
#include "alloc.h"
#include "rha_init.h"

// (1) symbols
$init_c_symbols

// (2) prototypes
$init_c_prototypes

// (3) type objects
object_t void_obj;
$init_c_typeobjects

// (4) ptype names
string_t ptype_names[] = {
$init_c_ptypenames
};

// (5) functions
$init_c_functions

// (6) init
#define ADD_TYPE(ttt, TTT)   \\
  setptype(ttt ## _proto, TTT ## _T);\\
  ttt ## _obj = new();\\
  assign(root, ttt ## _sym, ttt ## _obj);\\
  assign(ttt ## _obj, proto_sym, ttt ## _proto);

void add_function(object_t module, symbol_t s, object_t (*code)(tuple_t), int narg, ...)
{
  // create a struct containing all info about the builtin function
  fn_t f = ALLOC_SIZE(sizeof(struct _fn_t_));
  f->code = code;
  f->narg = narg;
  f->argptypes = ALLOC_SIZE(narg*sizeof(enum ptypes));

  // read out the argument types
  va_list ap;
  va_start(ap, narg);
  for (int i=0; i<narg; i++)
    f->argptypes[i] = va_arg(ap, int_t);
  va_end(ap);

  // create a new object
  object_t o = wrap_ptr(FN_T, fn_proto, f);

  // finally add it to module
  assign(module, s, o);
}

#define ADD_MODULE(mmm)   \\
  module = new();\\
  assign(modules, mmm ## _sym, module);

object_t rha_init()
{
  // (6.1) create prototypes (TYPES)
$init_c_init_prototypes

  // (6.2) create symbols (SYMBOLS, TYPES, MODULES, functions)
$init_c_init_symbols

  object_t root = new();
  assign(root, root_sym, root);
  assign(root, local_sym, root); // the outer-most local scope

  // (6.3) create type objects (TYPES)
$init_c_init_typeobjects

  // (6.4) create the void object
  void_obj = new();
  assign(root, void_sym, void_obj);


  // (6.5) add modules (MODULES, functions)
  object_t modules = new();
  assign(root, modules_sym, modules);
  object_t module = 0;
$init_c_add_modules

  return root;
}
ENDE
}

#####END#OF#FILE##########
