#!/usr/bin/env perl

use strict 'vars';
use warnings;

# input file:
#     rha_config.d
#
# output files:
#     rha_init.h
#     rha_init.c
#     rha_types.h

# NEXT STEPS
# * use new 'subs' for the usual include
# * move all type stuff to other places


# TODO:
# *   allow header files to define besides functions also variables
#     e.g. for exceptions
# *   automatically ensure the right order of modules
# *   remove the module name from the beginning of function names (MAYBE)


#################################
### step 1: preliminary stuff ###
#################################

my $conf_d_fname = "rha_config.d";
my $init_h_fname = "rha_init.h";
my $init_c_fname = "rha_init.c";
my $type_h_fname = "rha_types.h";

my $debug = shift @ARGV;

# C identifier
my $id = '[a-zA-Z_]\w*';

my $keyword = "_rha_";

# load def file into one string
open(FILE, "<$conf_d_fname") or die "Can't open $conf_d_fname: $!";
my @input = <FILE>;
close(FILE);
my $input = join '', @input;

# parse the def file
$input =~ /\/\/MODULES\s*\n(.*)\/\/TYPES\s*\n(.*)\/\/SYMBOLS\s*\n(.*)\/\/MACROS\s*\n(.*)\/\/EXPERIMENTAL\s*\n(.*)/s;
my $modules = $1;
my $types   = $2;
my $symbols = $3;
my $macros  = $4;
my $experim = $5;

# (0) remove comments
$modules =~ s/\/\/[^\n]*//g;
$types   =~ s/\/\/[^\n]*//g;
$symbols =~ s/\/\/[^\n]*//g;
$macros  =~ s/\/\/[^\n]*//g;
$experim =~ s/\/\/[^\n]*//g;

# (1) disect the includes
my @mods = $modules =~ /include\s+\"($id)\.h\"/gox;

# (2) disect the typedefs
my @tdefs = $types =~ /$keyword\s+($id)/gox;

# (3) disect the symbols
my @symbs = $symbols =~ /($id)\_sym/gox;

# (4) create a hash of types
my %typeset = map { $_ => 1 } @tdefs;


my %typenames = ();

# (5) the experimental stuff
# some of the types are not stored in the void pointer but are stored
# directly in the union that is attached to each object.  if a type
# should be stored directly it must appear in the following list,
# which should go ultimatively to rha_config.d
# all other types are assumed to be pointer and are stored as void
# pointers. 
my %typemapping = ( bool      => "int",
		    int       => "int",
		    bool_t    => "int",
		    int_t     => "int",
		    symbol_t  => "int",
		    size_t    => "int",
		    float     => "float",
		    double    => "real",
		    real_t    => "real",
		    builtin_t => "builtin_t" );

# (6) define regular expressions for C-header parsing

# regexp for 'const'
my $reco = "const\\s+";

# regexp for stars
my $rest = "\\**\\s*";

# regexp for brackets
my $rebr = "\\[\\s*\\]\\s*";

# regexp for typed function name
my $refn = "$id\\s*"     # type identifier
    ."(?:$rest)?"     # optional stars
    ."$id\\s*";       # function name

# regexp for typed argument
my $rear = "(?:"
    ."(?:$reco)?"     # optional 'const'
    ."$id\\s*"        # type identifier
    ."(?:$rest)?"     # optional stars
    ."(?:$id\\s*)?"   # optional identifier
    ."(?:$rebr)?"     # optional array brackets '[ ]'
    ."|\.\.\."        # alternatively '...'
    .")";

# regexp for argument list
my $reargs = "$rear\\s*" # first arg
    ."(?:"            # more args begin here
    .",\\s*"          # arg separating comma
    ."$rear"          # next arg
    .")*\\s*";        # more args are optional

# regexp for function declaration
my $refd = "($refn)\\s*" # return type and function name
    ."\\(\\s*"        # left bracket of args
    ."($reargs)"      # optional args
    ."\\)\\s*"        # right bracket of args
    .";";             # final semicolon



############################
# step 2: init all strings #
############################

my $type_h_types = $types;
my $type_h_ids = "";
my $type_h_prototypes = "";
my $type_h_typeobjects = "";
my $type_h_symbols = "";
my $type_h_macros = $macros;

my $init_h_modules = $modules;

my $init_c_symbols = "";
my $init_c_prototypes = "";
my $init_c_typeobjects = "";
my $init_c_ptypenames = "";
my $init_c_functions = "";
my $init_c_init_prototypes = "";
my $init_c_extend_prototypes = "";
my $init_c_init_symbols = "";
my $init_c_init_typeobjects = "";
my $init_c_add_pchecks = "";
my $init_c_add_modules = "";
my $init_c_add_functions = "";


##############################
# step 3: create all strings #
##############################


if (1) {
    &process_experimental();
}

# first the more complicated stuff: the functions
foreach my $module (@mods) {
    # add some stuff
    $init_c_add_modules .= "  ADD_MODULE($module);\n";
    $init_c_add_functions .= "  module = lookup(modules, $module"."_sym);\n";
    # also add a symbol
    push(@symbs, $module);

    # load the header file
    my $module_fname = $module.".h";
    open(FILE, "<$module_fname") or die "Can't open $module_fname: $!";
    my @input = <FILE>;
    close(FILE);
    $input = join '', @input;
    $input =~ s/\/\/[^\n]*//g;  # remove comments

    # parse it
    while ($input =~ /$keyword\s+($id)\s+($id)\s*\(([^\)]*)\)\s*;/gox) {
	my $fntype = $1;
	my $fnname = $2;
	my $fnargs = $3;
	print "matched: $fntype $fnname $fnargs\n" if $debug;
	
       	# disect the args (also get ellipses (...))
	my @args = $fnargs =~ /($id|\.\.\.)[^,]*/gox;
	my $narg = $#args + 1;
	
	# check whether all types appear in '%typeset'
	if (!$typeset{$fntype} && ($fntype ne "void")) {
	    die "type error: return type '$fntype' of '$fnname' in '$module_fname' not in 'rha_config.d'\n";
	}

	my $ellipses = 0; # == false
	foreach my $arg (@args) {
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
	$init_c_functions .= "any_t b_$fnname(tuple_t t) {\n";
	my $fncall_str = "$fnname(";
	
	if ($ellipses) {
	    # we ignore the second last argument which is
	    # e.g. something like "int_t narg"
	    my $dots = pop(@args);
	    my $dummy = pop(@args);
	    push(@args, $dots);
	    # adjust $narg, take off for 'int_t narg' and one for '...'
	    $narg = $narg - 2;
            # the b_* function needs additional code
            $init_c_functions .= "  int_t tlen = tuple_len(t);\n";
            $init_c_functions .= "  tuple_t args = tuple_new(tlen-$narg);\n";
            $init_c_functions .= "  for (int i=$narg; i<tlen; i++)\n";
            $init_c_functions .= "    tuple_set(args, i-$narg, tuple_get(t, i));\n";
	    # instead of calling the function itself, we call the
	    # function with the same name with a prepended 'v'
	    $fncall_str = "v" . $fncall_str;
	}
	my $start = 0;
	my $i = $start;
	foreach my $item (@args) {
	    if ($item eq "void") { die "'void' is not allowed as argument" }
	    if ($i>$start) { $fncall_str .= ", " }
	    my $fnarg_str = "tuple_get(t, $i)";
	    my $ucitem = uc($item);
	    if ($item eq "int_t") { $fnarg_str = "UNWRAP_INT($fnarg_str)" }
	    elsif ($item eq "bool_t") { $fnarg_str = "UNWRAP_BOOL($fnarg_str)" }
	    elsif ($item eq "symbol_t") { $fnarg_str = "UNWRAP_SYMBOL($fnarg_str)" }
	    elsif ($item eq "real_t") { $fnarg_str = "UNWRAP_REAL($fnarg_str)" }
	    elsif ($item eq "builtin_t") { $fnarg_str = "UNWRAP_BUILTIN($fnarg_str)" }
	    elsif ($item eq "...") {$fnarg_str = "args" }
	    elsif ($item ne "any_t") { $fnarg_str = "UNWRAP_PTR($ucitem, $fnarg_str)" }
	    $fncall_str .= $fnarg_str;
	    $i++;
	}
	$fncall_str .= ")";
	if ($fntype eq "int_t") { $fncall_str = "WRAP_INT($fncall_str)" }
	elsif ($fntype eq "bool_t") { $fncall_str = "WRAP_BOOL($fncall_str)" }
	elsif ($fntype eq "symbol_t") { $fncall_str = "WRAP_SYMBOL($fncall_str)" }
	elsif ($fntype eq "real_t") { $fncall_str = "WRAP_REAL($fncall_str)" }
	elsif ($fntype eq "builtin_t") { $fncall_str = "WRAP_BUILTIN($fncall_str)" }
	elsif ($fntype ne "any_t" && $fntype ne "void") {
	    my $ucfntype = uc($fntype);
	    $fncall_str = "WRAP_PTR($ucfntype, $fncall_str)";
	}
	if ($fntype eq "void") { $init_c_functions .= "  $fncall_str;\n  return void_obj;\n" }
	else { $init_c_functions .= "  return $fncall_str;\n" }
	$init_c_functions .= "}\n";

	# add code to add functions
	$init_c_add_functions .= "  add_function(module, $fnname"."_sym";
	$init_c_add_functions .= ", b_$fnname";
	if ($ellipses) {
	    # note that negative 'narg' will mean, at least 'narg' arguments
	    $init_c_add_functions .= ", true";
	}
	else {
	    $init_c_add_functions .= ", false";
	}
	$init_c_add_functions .= ", $narg";
	foreach my $item (@args) {
	    if ($item ne "...") {
		my $ucitem = uc($item);
		$init_c_add_functions .= ", $ucitem";
	    }
	}
	$init_c_add_functions .= ");\n";
    }

    # check for $module_init function
    if ($input =~ /$module\_init/) {
	if ($input =~ /void\s+$module\_init\(any_t\s*(\s$id\s*)?,\s*any_t\s*(\s$id\s*)?\)/) {
	    print $module, "_init(any_t, any_t) found\n" if $debug;
	    $init_c_add_functions .= "  $module"."_init(root, module);\n";
	}
	else {
	    die "type error, correct signature: 'void $module"."_init(any_t, any_t)'";
	}
    }
    $init_c_add_functions .= "\n";
}
chop($init_c_add_functions);
chop($init_c_add_functions);

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

sub process_experimental() {
    print "$experim\n" if $debug;

    # get rid of all newlines
    $experim =~ s/\n//gso;

    # replace white space by a single space (for readability)
    $experim =~ s/\s+/ /gso;

    # get rid of all stuff in curlied brackets
    while ($experim =~ s/{[^{}]*}/HUHU/gso) {};

    print "$experim\n";

    my $module = "experimental";
    $init_c_add_modules .= "  ADD_MODULE($module);\n";
    $init_c_add_functions .= "  module = lookup(modules, $module"."_sym);\n";
    # also add a symbol
    push(@symbs, $module);

    while ($experim =~ /$refd/go) {
	my $fntid  = $1;
	my $fnargs = $2;
	print "matched: $fntid with args $fnargs\n" if $debug;
	process_fun($fntid, $fnargs);
    }
    $init_c_add_functions .= "\n";
}


sub process_typedid {
    # take a typed identifier and returns a typename
    # note that this both should work for functionnames and args
    # stars and [] are resolved and add "_ptr"

    # note on types:
    # 

    my $tid = shift;
    $tid =~ s/\s+\*/\*/go;  # remove whitespace preceeding stars
    $tid =~ /(?:$reco)?($id\**|\.\.\.)\s*($id)?/o;
    my $typename = $1;
    my $varname  = $2;
    $typename =~ s/(\*|$rebr)/_ptr/go;
    print "found: $typename   $varname\n" if $debug;

    # take a note about the typename
    $typenames{$typename} = $typename;

    return ($typename, $varname);
}

sub process_fun() {
    # accesses $fntid and $fnargs
    my $fntid = shift;
    my $fnargs = shift;

    my @fnargs = split /\s*,\s*/, $fnargs;

    print "$fntid\n" if $debug;
    my ($fntype, $fnname) = process_typedid($fntid);
    
    print "fntype == $fntype; fnname == $fnname\n" if $debug;

    my @args = ();
    foreach my $fnarg (@fnargs) {
	print "$fnarg\n" if $debug;
	my ($typename, $varname) = process_typedid($fnarg);
	print "typename == $typename; varname == $varname\n" if $debug;
	push @args, $typename;
    }
    my $narg = $#args + 1;

    # are we having an ellipses?
    my $ellipses = 0; # == false
    foreach my $arg (@args) {
	if ($arg eq "...") {
	    # note that we don't care here in 'rha_config.pl'
	    # whether the ellipses is at the last argument
	    # position, because the C-compiler will complain
	    # about it...
	    $ellipses = 1; # == true
	}
    }

    # are we having 'void' as an argument?
    my $voidargs = 0;
    foreach my $arg (@args) {
	if ($arg eq "void") {
	    if (@args != 1)  {
		die "'void' must be only element of argument list";
	    }
	    $voidargs = 1;
	}
    }
    if ($voidargs) {
	# remove it
	@args = ();
    }

    # add a symbol for the function
    push(@symbs, $fnname);
    
    # add wrapper code
    $init_c_functions .= "any_t b_$fnname(tuple_t t) {\n";
    my $fncall_str = "$fnname(";
    
    if ($ellipses) {
	# we ignore the second last argument which is
	# e.g. something like "int_t narg"
	my $dots = pop @args;
	my $dummy = pop @args;
	push @args, $dots;
	# adjust $narg, take off for 'int_t narg' and one for '...'
	my $narg = $narg - 2;
	# the b_* function needs additional code
	$init_c_functions .= "  int_t tlen = tuple_len(t);\n";
	$init_c_functions .= "  tuple_t args = tuple_new(tlen-$narg);\n";
	$init_c_functions .= "  for (int i=$narg; i<tlen; i++)\n";
	$init_c_functions .= "    tuple_set(args, i-$narg, tuple_get(t, i));\n";
	# instead of calling the function itself, we call the
	# function with the same name with a prepended 'v'
	$fncall_str = "v" . $fncall_str;
    }
    my $start = 0;
    my $i = $start;
    my $item;
    foreach $item (@args) {
	if ($i>$start) { $fncall_str .= ", " }
	my $fnarg_str = "tuple_get(t, $i)";
	if ($typemapping{$item}) {
	    my $ucitem = uc($typemapping{$item});
	    $fnarg_str = "UNWRAP_$ucitem($fnarg_str)";
	}
	elsif ($item eq "...") {
	    $fnarg_str = "args";
	}
	else {
	    my $ucitem = uc($item)."_T";
	    $fnarg_str = "UNWRAP_PTR($ucitem, $fnarg_str)";
	}
	$fncall_str .= $fnarg_str;
	$i++;
    }
    $fncall_str .= ")";
    if ($typemapping{$item}) {
	my $ucitem = uc($typemapping{$item});
	$fncall_str = "WRAP_$ucitem($fncall_str)";
    }
    elsif ($fntype ne "any_t" && $fntype ne "void") {
	my $ucfntype = uc($fntype)."_T";
	$fncall_str = "WRAP_PTR($ucfntype, $fncall_str)";
    }
    if ($fntype eq "void") { 
	$init_c_functions .= "  $fncall_str;\n  return void_obj;\n" 
    }
    else { 
	$init_c_functions .= "  return $fncall_str;\n" 
    }
    $init_c_functions .= "}\n";
    
    # add code to add functions
    $init_c_add_functions .= "  add_function(module, $fnname"."_sym";
    $init_c_add_functions .= ", b_$fnname";
    if ($ellipses) {
	$init_c_add_functions .= ", true";
    }
    else {
	$init_c_add_functions .= ", false";
    }
    $init_c_add_functions .= ", $narg";
    foreach my $item (@args) {
	if ($item ne "...") {
	    my $ucitem = uc($item)."_T";
	    $init_c_add_functions .= ", $ucitem";
	}
    }
    $init_c_add_functions .= ");\n";
}


sub create_ids {
    $type_h_ids .= "  VOID_T = 0";
    $init_c_ptypenames .= "    \"void\"";
    my $id = 1;
    foreach my $item (@tdefs) {
	my $ucitem = uc($item);
	my $iitem = substr($item, 0, -2);
	$type_h_ids .= ",\n  $ucitem = $id";
	$init_c_ptypenames .= ",\n    \"$iitem\"";
	$id++;
    }
}

sub create_prototypes {
    my $ntdefs = scalar(@tdefs) + 1;
    $type_h_prototypes .= "extern any_t prototypes[$ntdefs];\n";
    $type_h_prototypes .= "extern any_t fn_data_proto;\n";
    $type_h_prototypes .= "extern any_t implementation_proto;\n";
    $type_h_prototypes .= "extern any_t pattern_proto;\n";
    $init_c_prototypes .= "any_t prototypes[$ntdefs];\n";
    $init_c_prototypes .= "any_t fn_data_proto = 0;\n";
    $init_c_prototypes .= "any_t implementation_proto = 0;\n";
    $init_c_prototypes .= "any_t pattern_proto = 0;\n";
    $type_h_typeobjects .= "extern any_t typeobjects[$ntdefs];\n";
    $init_c_typeobjects .= "any_t typeobjects[$ntdefs];\n";
    $init_c_init_prototypes .= "  prototypes[0] = 0; // void prototype\n";
    $init_c_init_prototypes .= "  for (int i = 1; i < $ntdefs; i++) {\n";
    $init_c_init_prototypes .= "    prototypes[i] = create_pt(i);\n";
    $init_c_init_prototypes .= "    typeobjects[i] = new();\n";
    $init_c_init_prototypes .= "  }\n";
    $init_c_init_prototypes .= "  typeobjects[ANY_T] = 0;\n";
    my $i = 1;
    foreach my $item (@tdefs) {
	my $ucitem = uc($item);
	my $iitem = substr($item, 0, -2);
	my $uciitem = uc($iitem);
	if ($item ne "any_t") {
	    $init_c_init_typeobjects .= "  ADD_TYPE($iitem, $uciitem);\n";
	    $init_c_add_pchecks .= "  assign(typeobjects[$ucitem], check_sym, pcheck_f);\n";
	    # note that we omit ANY_T for the type slot
	    $init_c_extend_prototypes .= "  assign(prototypes[$i], type_sym, typeobjects[$i]);\n";
	}
	else{
	    $init_c_extend_prototypes .= "  //assign(prototypes[$i], type_sym, typeobjects[$i]); // omitted on purpose\n";

	}
	$i = $i+1;
    }
}

sub create_symbols {
    # add all @tdefs to @symbs
    foreach my $item (@tdefs) {
	push(@symbs, substr($item, 0, -2));
    }

    # set up flags for duplicate removal
    my %printed;
    for (@symbs) { $printed{$_} = 0 }
    foreach my $item (@symbs) {
	if ($printed{$item} == 0) {
	    $type_h_symbols .= "extern symbol_t $item"."_sym;\n";
	    $init_c_symbols .= "symbol_t $item"."_sym;\n";
	    $init_c_init_symbols .= "  $item"."_sym = symbol_new(\"$item\");\n";
	    $printed{$item} = 1;
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
extern any_t void_obj;
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
extern any_t rha_init();
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
any_t void_obj = 0;
$init_c_typeobjects

// (4) ptype names
string_t ptype_names[] = {
$init_c_ptypenames
};

// (5) functions
$init_c_functions

// (6) init
#define ADD_TYPE(ttt, TTT)                                            \\
  assign(root, ttt ## _sym, typeobjects[TTT ## _T]);                  \\
  assign(typeobjects[TTT ## _T], proto_sym, prototypes[TTT ## _T]);   \\
  assign(typeobjects[TTT ## _T], parent_sym, type_obj);               \\
  assign(typeobjects[TTT ## _T], name_sym, WRAP_PTR(STRING_T, #ttt));

#define ADD_MODULE(mmm)                                               \\
  module = new();                                                     \\
  assign(modules, mmm ## _sym, module);

static
void add_function(any_t module, symbol_t s, 
                  any_t (*code)(tuple_t),
                  bool_t varargs, int narg, ...)
{
  va_list args;
  va_start(args, narg);
  any_t f = vcreate_builtin(code, varargs, narg, args);
  va_end(args);

  assign(module, s, f);
}

static
any_t create_special_fn_data(any_t module, bool_t method, 
                                symbol_t fn_sym, int_t narg, ...)
{
  // "..." must be pairs of 'any_t' and 'symbol_t'
  tuple_t signature = 0;
  list_t fnbody_l = 0;
  if (narg < 0) {
    // rhabarber function with variable argument list
    signature= tuple_new(1);
    tuple_set(signature, 0, create_pattern(1, WRAP_SYMBOL(symbol_new("..."))));
    fnbody_l = list_new();
    list_append(fnbody_l, WRAP_SYMBOL(symbol_new("args")));
  }
  else {
    va_list ap;
    va_start(ap, narg);
    signature = tuple_new(narg);
    fnbody_l = list_new();
    for (int i = 0; i < narg; i++) {
      any_t thetype = va_arg(ap, any_t);
      any_t theliteral = WRAP_SYMBOL(va_arg(ap, symbol_t));
      assert(theliteral);
      if (thetype) {
	tuple_set(signature, i, create_pattern(2, theliteral, thetype));
      }
      else {
	tuple_set(signature, i, create_pattern(1, theliteral));
      }
      list_append(fnbody_l, theliteral);
    }
    va_end(ap);
    // are we creating a method?
    if (method) {
      list_prepend(fnbody_l, WRAP_SYMBOL(this_sym));
    }
  }
  list_prepend(fnbody_l, WRAP_SYMBOL(fn_sym));
  return create_fn_data(module, signature, 
			WRAP_PTR(TUPLE_T, list_to_tuple(fnbody_l)));
}

any_t rha_init()
{
  // (6.0) prototypes (TYPES)
  fn_data_proto = new();
  implementation_proto = new();
  pattern_proto = new();
$init_c_init_prototypes

  // (6.1) create symbols (SYMBOLS, TYPES, MODULES, functions)
$init_c_init_symbols

  any_t root = new();
  assign(root, root_sym, root);
  assign(root, local_sym, root); // the outer-most local scope
  void_obj = new();
  assign(root, void_sym, void_obj);

  // (6.2) add modules
  any_t modules = new();
  assign(root, modules_sym, modules);
  any_t module = 0;
$init_c_add_modules

  // (6.3) create type objects (TYPES)
  any_t fn_data = 0;
  any_t type_obj = new();
  assign(root, type_sym, type_obj);
  assign(type_obj, proto_sym, type_obj);
  any_t pattern_obj = clone(type_obj);
  // note that for 'fn_data_proto' there is only the prototype but no
  // object in root called 'fn_data'.  otherwise 'root' would be callable!
  assign(root, symbol_new("fn_data_proto"), fn_data_proto);
  assign(root, symbol_new("implementation_proto"), implementation_proto);
  assign(root, symbol_new("pattern"), pattern_obj);
  assign(pattern_obj, proto_sym, pattern_proto);
$init_c_init_typeobjects
  // in rhabarber the following looks like this:
  //     type.check = fn (x) modules.check(this, x);  // note 'check'
  //     bool.check = fn (x) modules.pcheck(this, x); // note 'pcheck'
  //     int.check = fn (x) modules.pcheck(this, x);
  module = lookup(modules, symbol_new("object"));
  fn_data = create_special_fn_data(module, true, symbol_new("check"), 1,
                                   0, symbol_new("x"));
  assign(type_obj, check_sym, create_function(fn_data));
  fn_data = create_special_fn_data(module, true, symbol_new("pcheck"), 1,
		                   0, symbol_new("x"));
  any_t pcheck_f = create_function(fn_data);
$init_c_add_pchecks

  // (6.4) add type slots to the prototypes
$init_c_extend_prototypes

  // (6.5) add functions (functions)
$init_c_add_functions

  // (6.6) add more special functions to certain objects
  //     pattern(...) = vcreate_pattern(args);
  module = lookup(modules, symbol_new("core"));
  fn_data = create_special_fn_data(module, false, symbol_new("vcreate_pattern"), -1);
  assign(pattern_obj, fn_data_sym, fn_data);


  // (6.6) add essential stuff
  module = lookup(modules, symbol_new("object"));
  assign(root, symbol_new("assign"), lookup(module, symbol_new("assign")));
  assign(root, symbol_new("extend"), lookup(module, symbol_new("extend")));
  module = lookup(modules, symbol_new("core"));
  assign(root, symbol_new("map_fn"), lookup(module, symbol_new("map_fn")));

  return root;
}
ENDE
}

#####END#OF#FILE##########
