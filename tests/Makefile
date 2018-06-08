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
	$(MAKE) -C periodic_stats
	$(MAKE) -C prctl
	$(MAKE) -C rdpmc
	$(MAKE) -C record_sample
	$(MAKE) -C size
	$(MAKE) -C socket
	$(MAKE) -C sysfs
	$(MAKE) -C tracepoints
	$(MAKE) -C utils
	$(MAKE) -C x86_amd
	$(MAKE) -C x86_intel


install: all
	$(MAKE) -C attr_fields install
	$(MAKE) -C breakpoints install
	$(MAKE) -C bugs install
	$(MAKE) -C constraints install
	$(MAKE) -C corner_cases install
	$(MAKE) -C error_returns install
	$(MAKE) -C flags install
	$(MAKE) -C generalized_events install
	$(MAKE) -C ioctl install
	$(MAKE) -C multiplexing install
	$(MAKE) -C multithread install
	$(MAKE) -C openmp install
	$(MAKE) -C overflow install
	$(MAKE) -C periodic_stats install
	$(MAKE) -C prctl install
	$(MAKE) -C rdpmc install
	$(MAKE) -C record_sample install
	$(MAKE) -C size install
	$(MAKE) -C socket install
	$(MAKE) -C sysfs install
	$(MAKE) -C tracepoints install
	$(MAKE) -C utils install
	$(MAKE) -C x86_amd install
	$(MAKE) -C x86_intel install


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
	$(MAKE) -C periodic_stats clean
	$(MAKE) -C prctl clean
	$(MAKE) -C rdpmc clean
	$(MAKE) -C record_sample clean
	$(MAKE) -C size clean
	$(MAKE) -C socket clean
	$(MAKE) -C sysfs clean
	$(MAKE) -C tracepoints clean
	$(MAKE) -C utils clean
	$(MAKE) -C x86_amd clean
	$(MAKE) -C x86_intel clean

clean-local:
	@- $(RM) *~
