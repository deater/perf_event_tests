all:
	$(MAKE) -C attr_fields
	$(MAKE) -C breakpoints
	$(MAKE) -C bugs
	$(MAKE) -C constraints
	$(MAKE) -C corner_cases
	$(MAKE) -C error_returns
	$(MAKE) -C flags
	$(MAKE) -C generalized_events
	$(MAKE) -C ioctl
	$(MAKE) -C multiplexing
	$(MAKE) -C multithread
	$(MAKE) -C openmp
	$(MAKE) -C overflow
	$(MAKE) -C rdpmc
	$(MAKE) -C record_sample
	$(MAKE) -C socket
	$(MAKE) -C sysfs
	$(MAKE) -C tracepoints
	$(MAKE) -C utils
	$(MAKE) -C x86_intel

clean: clean-local
	$(MAKE) -C attr_fields clean
	$(MAKE) -C breakpoints clean
	$(MAKE) -C bugs clean
	$(MAKE) -C constraints clean
	$(MAKE) -C corner_cases clean
	$(MAKE) -C error_returns clean
	$(MAKE) -C flags clean
	$(MAKE) -C generalized_events clean
	$(MAKE) -C ioctl clean
	$(MAKE) -C multiplexing clean
	$(MAKE) -C multithread clean
	$(MAKE) -C openmp clean
	$(MAKE) -C overflow clean
	$(MAKE) -C rdpmc clean
	$(MAKE) -C record_sample clean
	$(MAKE) -C socket clean
	$(MAKE) -C sysfs clean
	$(MAKE) -C tracepoints clean
	$(MAKE) -C utils clean
	$(MAKE) -C x86_intel clean

clean-local:
	@- $(RM) *~
