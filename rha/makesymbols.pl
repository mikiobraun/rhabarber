#!/usr/bin/env perl

if ($#ARGV == -1) {
    die "Usage: newtr.pl name\n";
}

$symtab = shift @ARGV;
if ($symtab =~ m/.*\/(.*)/) {
    $symtab = $1;
}
$SYMTAB = "\U$symtab";

$decl = "";
$def = "";
$init = "";
$add = "";
#
# read the input file and collect the symbols
#

open IN, "<$symtab" || die "Can't open $symtab!";

while(<IN>) {
    if(/\/\//) {
	$decl .= "$_";
	$def .= "$_";
	$init .= "  $_";
	$add .= "  $_";
    }
    elsif(/([0-9a-zA-Z_]+)\s+([^ \t\n]+)/) {
	$symname = $1;
	$symname_sym = $1 . "_sym";
	$symtext = $2;

	$decl .=  "extern symbol_tr $symname_sym;\n";
	$def  .=  "symbol_tr $symname_sym = 0;\n";
	$init .= "  $symname_sym = symbol_new(\"$symtext\");\n";
	$add .= "  object_assign(root, symbol_new(\"$symname_sym\"), $symname_sym);\n";
    }
    else {
	$decl .=  "\n";
	$def .=  "\n";
	$init .=  "\n";
	$add .=  "\n";
    }
}
close IN;

#
# print the result
#

open OUT, ">$symtab.h" || die "Can't open $symtab.h!";

print OUT <<ENDE;
/*
 * $symtab.h - Symbol table declarations, definitions and assignments
 *
 * This file has been generated automatically with makesymbols.pl
 * from the file $symtab.
 *
 * DO NOT EDIT THIS FILE! Edit $symtab instead.
 *
 */

/*
 * Usage:
 * 
 * you can include this file three times, by defining
 *
 * $SYMTAB\_DECLARE - extern declarations of the symbols
 * $SYMTAB\_DEFINE  - define the symbols
 * $SYMTAB\_INIT    - initialize the symbols
 * $SYMTAB\_ADD     - add the symbols to the symbol table of root
 *
 * This should be done like this:
 *
 * #define $SYMTAB\_DECLARE
 * #include "$symtab.h"
 * #undef $SYMTAB\_DECLARE
 *
 * etc.
 */

#ifdef $SYMTAB\_DECLARE
#line 1 "$symtab"
$decl#endif

#ifdef $SYMTAB\_DEFINE
#line 1 "$symtab"
$def#endif

#ifdef $SYMTAB\_INIT
#line 1 "$symtab"
$init#endif

#ifdef $SYMTAB\_ADD
#line 1 "$symtab"
$add#endif

ENDE

close OUT;

print <<ENDE;
makesymbol.pl completed.

To declare the symbols (in a .h-file), insert

#define $SYMTAB\_DECLARE
#include "$symtab.h"
#undef $SYMTAB\_DECLARE

To define the symbols (at the top of a .c-file), insert

#define $SYMTAB\_DEFINE
#include "$symtab.h"
#undef $SYMTAB\_DEFINE

To initialize the symbols (somewhere in the .c-file), insert

#define $SYMTAB\_INIT
#include "$symtab.h"
#undef $SYMTAB\_INIT

To add the symbols (somewhere in the .c-file), insert

#define $SYMTAB\_ADD
#include "$symtab.h"
#undef $SYMTAB\_ADD

This little hack has been written by Mikio L. Braun, mikiobraun\@gmail.com
ENDE
