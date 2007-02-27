#!/usr/bin/env perl

if ($#ARGV == -1) {
    die "Usage: newtr.pl name\n";
}

$name = shift @ARGV;
$name_s = $name . "_s";
$name_tr = $name . "_tr";
$name_type = $name . "_type";
$NAME = "\U$name";
$NAME_H = $NAME . "_H";

$hname = "$name\_tr.h";
$cname = "$name\_tr.c";

if (-r $hname || -r $cname) {
    die "Files already exist! Please delete files first manually!\n";
}

open HFILE, ">$hname" || die "Can't open $hname!";
open CFILE, ">$cname" || die "Can't open $cname!";

print HFILE <<ENDE;
/*
 * $name - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef $NAME_H
#define $NAME_H

#include "primtype.h"
#include "object.h"

typedef struct $name_s *$name_tr;

extern primtype_t $name_type;

extern void $name\_init(void);

extern constructor $name_tr $name\_new();

#endif
ENDE
close HFILE;

print CFILE <<ENDE;
/*
 * $name - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "$name_tr.h"

#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"

struct $name_s
{
  RHA_OBJECT;
  /* add more entries here */
};

#define CHECK(x) CHECK_TYPE($name, x)

primtype_t $name_type = 0;

void $name\_init(void)
{
  if (!$name_type) {
    $name_type = primtype_new("$name", sizeof(struct $name_s));
    /* add more initialization here */
    (void) $name\_new();
  }
}

$name_tr $name\_new()
{
  $name_tr new$name = object_new($name_type);
  return new$name;
}

#undef CHECK
ENDE
close CFILE;

print <<ENDE;
Done. Things for you left to do (I'd rather not touch that files):

In file Makefile: 

  Add rule

  $name\_stub.h $name\_stub.c : $name\_tr.h rha_stub.pl
	  rha_stub.pl $name\_tr.h

  Add $name\_tr.o to the variable TR in the Makefile.

In file rhabarber.c: Add

    #include "$name_tr.h"
    #include "$name\_stub.h"

  Add to function init():

    $name\_init();

  Add to function initstubs():

    $name\_stub_init(root);


Don't forget to "make stubs"

This nearly trivial perl-script has been brought to you by
Mikio Braun, mikio\@first.fhg.de

ENDE
