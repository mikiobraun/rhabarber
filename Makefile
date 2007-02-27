SHELL=/bin/bash
DIR=$(PWD)
GCVERSION=gc6.8

initialize:
	[ -d include ] || mkdir include
	[ -d include/ctmpl ] || mkdir include/ctmpl
	[ -d bin ] || mkdir bin
	[ -d lib ] || mkdir lib
	(cd ctmpl; make depend; make maketmpl)
	(cd ctmpl; make install)
	(cd util; make depend; make; make install)
	if [ \! -f lib/libgc.a ];\
	then \
		(cd $(GCVERSION); ./configure --prefix=$(DIR) );\
		(cd $(GCVERSION); make ; make install ) &>makegc.log;\
	fi
	(cd rha; make templates core_symbols.h depend; make)
	@echo
	@echo "Congratulations! Compilation complete."
	@echo
	@echo "Don't forget to set LD_LIBRARY_PATH to include $(PWD)/lib."

clean:
	(cd ctmpl; make clean)
	(cd rha; make clean)
	(cd util; make clean)
	(cd $(GCVERSION); make clean)

distclean:
	rm -rf bin
	rm -rf include
	rm -rf lib
	rm -rf share
	(cd $(GCVERSION); make distclean)
	(cd ctmpl; make distclean)
	(cd rha; make distclean)
	(cd util; make distclean)
	rm makegc.log

