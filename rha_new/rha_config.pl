#!/usr/bin/env perl

# input file:
#     rha_config.d
#
# output files:
#     rha_init.h
#     rha_init.c
#     rha_types.h

# TODO:
# *   allow header files to define besides functions also variables
#     e.g. for exceptions
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
	
       	# disect the args (also get ellipses (...))
	@args = $fnargs =~ /($id|\.\.\.)[^,]*/gox;
	$narg = $#args + 1;
	
	# check whether all types appear in '%typeset'
	if (!$typeset{$fntype} && ($fntype ne "void")) {
	    die "type error: return type '$fntype' of '$fnname' in '$module_fname' not in 'rha_config.d'\n";
	}

	$ellipses = 0; # == false
	foreach $arg (@args) {
	    if (!$typeset{$arg}) {
		if ($arg eq "...") {
		    # note that we don't care here in 'rha_config.pl'
		    # whether the ellipses is at the last argument
		    # position, because the C-compiler will complain
		    # about it...
		    $ellipses = 1; # == true
		}
		else {
		    die "type error: arg type '$arg' of '$fnname' in '$module_fname' not in 'rha_config.d'\n";
		}
	    }
	}
	# add a symbol for the function
	push(@symbs, $fnname);
	
	# add wrapper code
	$init_c_functions .= "object_t b_$fnname(tuple_t t) {\n";
	$fncall_str = "$fnname(";
	
	if ($ellipses) {
	    # we ignore the second last argument which is
	    # e.g. something like "int_t narg"
	    $dots = pop(@args);
	    $dummy = pop(@args);
	    push(@args, $dots);
	    # adjust $narg, take off for 'int_t narg' and one for '...'
	    $narg = $narg - 2;
	    # the b_* function needs additional code
	    $init_c_functions .= "  int_t tlen = tuple_len(t);\n";
	    $init_c_functions .= "  list_t l = list_new();\n";
	    $init_c_functions .= "  for (int i=$narg+1; i<tlen; i++)\n";
	    $init_c_functions .= "    list_append(l, tuple_get(t, i));\n";
	    # instead of calling the function itself, we call the
	    # function with the same name with a prepended 'v'
	    $fncall_str = "v" . $fncall_str;
	}
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
	    elsif ($item eq "...") {$fnarg_str = "tlen-$narg-1, l" }
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
	    $fncall_str = "WRAP_PTR($ucfntype, $fncall_str)";
	}
	if ($fntype eq "void") { $init_c_functions .= "  $fncall_str;\n  return 0;\n" }
	else { $init_c_functions .= "  return $fncall_str;\n" }
	$init_c_functions .= "}\n";

	# add code to add functions
	$init_c_add_modules .= "  add_function(module, $fnname"."_sym";
	$init_c_add_modules .= ", b_$fnname";
	if ($ellipses) {
	    # note that negative 'narg' will mean, at least 'narg' arguments
	    $init_c_add_modules .= ", true";
	}
	else {
	    $init_c_add_modules .= ", false";
	}
	$init_c_add_modules .= ", $narg";
	foreach $item (@args) {
	    if ($item ne "...") {
		$ucitem = uc($item);
		$init_c_add_modules .= ", $ucitem";
	    }
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
    $type_h_ids .= "  VOID_T = 0";
    $init_c_ptypenames .= "    \"void\"";
    $id = 1;
    foreach $item (@tdefs) {
	$ucitem = uc($item);
	$iitem = substr($item, 0, -2);
	$type_h_ids .= ",\n  $ucitem = $id";
	$init_c_ptypenames .= ",\n    \"$iitem\"";
	$id++;
    }
}

sub create_prototypes {
    foreach $item (@tdefs) {
	if ($item ne "object_t") {
	    $ucitem = uc($item);
	    $iitem = substr($item, 0, -2);
	    $uciitem = uc($iitem);
	    $type_h_typeobjects .= "extern object_t $iitem"."_obj;\n";
	    $init_c_typeobjects .= "object_t $iitem"."_obj;\n";
	    $init_c_init_typeobjects .= "  ADD_TYPE($iitem, $uciitem);\n";
	}
    }
    $ntdefs = scalar(@tdefs) + 1;
    $type_h_prototypes .= "extern object_t prototypes[$ntdefs];\n";
    $init_c_prototypes .= "object_t prototypes[$ntdefs];\n";
    $init_c_init_prototypes .= "  prototypes[0] = 0; // void prototype\n";
    $init_c_init_prototypes .= "  for (int i = 1; i < $ntdefs; i++) {\n";
    $init_c_init_prototypes .= "    prototypes[i] = create_pt(i);\n";
    $init_c_init_prototypes .= "  }";
}

sub create_symbols {
    # add all @tdefs to @symbs
    foreach $item (@tdefs) {
	push(@symbs, substr($item, 0, -2));
    }

    # set up flags for duplicate removal
    for (@symbs) { $printed{$_} = 0 }
    foreach $item (@symbs) {
	if ($printed{$item} == 0) {
	    $type_h_symbols .= "extern symbol_t $item"."_sym;\n";
	    $init_c_symbols .= "symbol_t $item"."_sym;\n";
	    $init_c_init_symbols .= "  $item"."_sym = symbol_new(\"$item\");\n";
	    $printed{$item} == 1;
	}
    }
}

#########################################

sub create_type_h {
    return <<ENDE;
// WARNING: don't edit here!  
// This file has been automatically generated by 'rha_config.pl'.
// Instead edit 'rha_config.d' (or 'rha_config.pl' if you must ;) )

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
#include <assert.h>
#include "alloc.h"
#include "eval.h"
#include "tuple_fn.h"
#include "list_fn.h"
#include "rha_init.h"

// (1) symbols
$init_c_symbols

// (2) prototypes
$init_c_prototypes

// (3) type objects
$init_c_typeobjects

// (4) ptype names
string_t ptype_names[] = {
$init_c_ptypenames
};

// (5) functions
$init_c_functions

// (6) init
#define ADD_TYPE(ttt, TTT)                                 \\
  ttt ## _obj = new();                                     \\
  assign(root, ttt ## _sym, ttt ## _obj);                  \\
  assign(ttt ## _obj, proto_sym, prototypes[TTT ## _T]);   \\
  assign(ttt ## _obj, parent_sym, type_obj);               \\
  assign(ttt ## _obj, name_sym, WRAP_PTR(STRING_T, #ttt));

#define ADD_MODULE(mmm)                                  \\
  module = new();                                        \\
  assign(modules, mmm ## _sym, module);

void add_function(object_t module, symbol_t s, 
                  object_t (*code)(tuple_t),
                  bool_t varargs, int narg, ...)
{
  va_list args;
  va_start(args, narg);
  object_t f = vcreate_function(code, varargs, narg, args);
  va_end(args);

  assign(module, s, f);
}

object_t rha_init()
{
  // (6.0) create parent symbol
  parent_sym = symbol_new("parent");

  // (6.1) create prototypes (TYPES)
$init_c_init_prototypes

  // (6.2) create symbols (SYMBOLS, TYPES, MODULES, functions)
$init_c_init_symbols

  object_t root = new();
  assign(root, root_sym, root);
  assign(root, local_sym, root); // the outer-most local scope

  // (6.3) create type objects (TYPES)
  object_t type_obj = new();
  assign(type_obj, name_sym, WRAP_PTR(STRING_T, "type"));
  assign(type_obj, proto_sym, type_obj);
  assign(root, type_sym, type_obj);
$init_c_init_typeobjects

  // (6.4) add modules (MODULES, functions)
  object_t modules = new();
  assign(root, modules_sym, modules);
  object_t module = 0;
$init_c_add_modules

  return root;
}
ENDE
}

#####END#OF#FILE##########
