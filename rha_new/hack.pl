#!/usr/bin/env perl

# automatically edit all source file...

#use strict 'vars';
use strict;
use warnings;

unshift(@ARGV, '-') unless @ARGV;
while ($ARGV = shift @ARGV) {
    unless (open(ARGV, $ARGV)) {
        warn "Can't open $ARGV: $!\n";
        next;
    }
    # load the whole file into a string
    print "reading from $ARGV\n";
    my $content = join '', <ARGV>;
    close(ARGV);

    # do the automatic editing

    # replace stuff like "_INT" with "RHA_INT"
    $content =~ s/([^\w])_([A-Z_]+)/$1RHA_$2/gox;
    
    # replace stuff like "RHA_INT" with "RHA_int"
    $content =~ s/([^\w])RHA_([A-Z_]+)/$1RHA_\L$2\E/gox;

    # replace pyobject_ptr with PyObject_ptr
    $content =~ s/RHA_pyobject_ptr/RHA_PyObject_ptr/gox;

    # write-out the file
    print "writing to $ARGV\n";
    open(ARGV, ">$ARGV");
    print ARGV $content;
    close(ARGV);
}
