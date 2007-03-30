function maketmpl {
    PREFIX=../..
    TMPLDIR=$PREFIX/ctmpl
    TMPLDEFS='-ialloc.h alloc=GC_malloc free=GC_free'
    MAKETMPL=$TMPLDIR/maketmpl

    TMPLFILE=$1; shift

    $MAKETMPL $TMPLDEFS $* $TMPLDIR/$TMPLFILE
}

maketmpl tuple.tmpl T=object_t -iobject.h 
maketmpl tuple.tmpl T=symbol_tr -isymbol_tr.h 

## for parse_tr.c
maketmpl tuple.tmpl T=int 
maketmpl tuple.tmpl T=treeintint_t -itreeintint.h 
maketmpl list.tmpl T=int
maketmpl list.tmpl T=voidp -ivoidp_defs.h
maketmpl tuple.tmpl T=voidp -ivoidp_defs.h
maketmpl tree.tmpl K=int V=int -iintdefs.h lessthan=int_lessthan
maketmpl tree.tmpl K=int V=voidp -iintdefs.h -ivoidp_defs.h lessthan=int_lessthan
maketmpl list.tmpl T=object_t -iobject.h
maketmpl hash.tmpl K=symbol_tr V=object_t -iobject.h -isymbol_tr.h compare=symbol_equal_symbol hash=symbol_hash
maketmpl tree.tmpl K=symbol_tr V=object_t -iobject.h -isymbol_tr.h lessthan=symbol_less_symbol
maketmpl tree.tmpl K=string_t V=int -istringdefs.h lessthan=string_lessthan
maketmpl tree.tmpl K=int V=string_t -iintdefs.h -istringdefs.h lessthan=int_lessthan
maketmpl tree.tmpl K=object_t V=object_t -ituple_tr.h lessthan=tuple_less_tuple
