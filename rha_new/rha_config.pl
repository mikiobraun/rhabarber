#!/usr/bin/env perl

# input file:
#     rha_stub.def
#
# output files:
#     rha_stub.h
#     rha_stub.c


#################################
### step 1: preliminary stuff ###
#################################

$basename = "rha_stub";
$dfile = "$basename.d";
$hfile = "$basename.h";
$cfile = "$basename.c";

$debug = shift @ARGV;

# C identifier
$id = '[a-zA-Z_]\w*';

# load def file into one string
open(DFILE, "<$dfile") or die "Can't open $dfile: $!";
@input = <DFILE>;
close(DFILE);
$input = join '', @input;

# parse the def file
$input =~ /\/\/INCLUDES(.*)\/\/DATATYPES(.*)/s;
$includes = $1;
$datatypes = $2;

# disect the includes
@incls = $includes =~ /include\s+\"($id)\.h\"/gox;

# disect the typedefs
while ($datatypes =~ /\s+\*?($id)\s*;\s*(\/\/.*)?\s*(typedef|\s)/gox) {
    print $1, "\n";
}
exit;

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
