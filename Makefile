include Makefile.globals.inc

SCRIPT_LIST = \
	      run_crash_tests.sh \
	      run_tests.sh

all:
	$(MAKE) -C lib
	$(MAKE) -C tests
	$(MAKE) -C crashes
	$(MAKE) -C fuzzer
	$(MAKE) -C tools
	$(MAKE) -C warnings

install: all
	$(MAKE) -C tests install
	$(MAKE) -C crashes install
	$(MAKE) -C fuzzer install
	$(MAKE) -C tools install
	$(MAKE) -C warnings install
	$(INSTALL) -m755 $(SCRIPT_LIST) $(prefix)

papi:
	$(MAKE) -C papi

clean: clean-local
	$(MAKE) -C lib clean
	$(MAKE) -C tests clean
	$(MAKE) -C papi clean
	$(MAKE) -C crashes clean
	$(MAKE) -C fuzzer clean
	$(MAKE) -C tools clean
	$(MAKE) -C warnings clean

clean-local:
	@- $(RM) *~
