all:
	$(MAKE) -C lib
	$(MAKE) -C tests
	$(MAKE) -C crashes
	$(MAKE) -C fuzzer
	$(MAKE) -C tools

papi:
	$(MAKE) -C papi

clean: clean-local
	$(MAKE) -C lib clean
	$(MAKE) -C tests clean
	$(MAKE) -C papi clean
	$(MAKE) -C crashes clean
	$(MAKE) -C fuzzer clean
	$(MAKE) -C tools clean

clean-local:
	@- $(RM) *~
