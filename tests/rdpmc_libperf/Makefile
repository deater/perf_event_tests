include ../../Makefile.globals.inc

DIR_LIBPERF = /usr/local/lib/libperf
INCLUDES = -I$(DIR_LIBPERF)/include
RPATH = -Wl,-rpath $(DIR_LIBPERF)/lib64
LPATH = -L$(DIR_LIBPERF)/lib64

CPPFLAGS += -I../../include $(INCLUDES)
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LDLIBS += -lpthread
LIB = ../../lib

PROGRAM_LIST = \
	rdpmc_attach \
	rdpmc_attach_cpu \
	rdpmc_attach_global_cpu \
	rdpmc_attach_other_cpu \
	rdpmc_attach_multi_enable \
	rdpmc_exec \
	rdpmc_exec_papi \
	rdpmc_group \
	rdpmc_group_syswide \
	rdpmc_multiattach \
	rdpmc_pthreads \
	rdpmc_support

all: $(PROGRAM_LIST)

rdpmc_attach: rdpmc_attach.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_attach.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_attach

rdpmc_attach.o: rdpmc_attach.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_attach.c

rdpmc_attach_cpu: rdpmc_attach_cpu.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_attach_cpu.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_attach_cpu

rdpmc_attach_cpu.o: rdpmc_attach_cpu.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_attach_cpu.c

rdpmc_attach_global_cpu: rdpmc_attach_global_cpu.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_attach_global_cpu.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_attach_global_cpu

rdpmc_attach_global_cpu.o: rdpmc_attach_global_cpu.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_attach_global_cpu.c

rdpmc_attach_other_cpu: rdpmc_attach_other_cpu.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_attach_other_cpu.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_attach_other_cpu

rdpmc_attach_other_cpu.o: rdpmc_attach_other_cpu.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_attach_other_cpu.c

rdpmc_attach_multi_enable: rdpmc_attach_multi_enable.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_attach_multi_enable.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_attach_multi_enable

rdpmc_attach_multi_enable.o: rdpmc_attach_multi_enable.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_attach_multi_enable.c

rdpmc_exec: rdpmc_exec.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_exec.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_exec

rdpmc_exec.o: rdpmc_exec.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_exec.c

rdpmc_exec_papi: rdpmc_exec_papi.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_exec_papi.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_exec_papi

rdpmc_exec_papi.o: rdpmc_exec_papi.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_exec_papi.c

rdpmc_group: rdpmc_group.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_group.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_group

rdpmc_group.o: rdpmc_group.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_group.c

rdpmc_group_syswide: rdpmc_group_syswide.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_group_syswide.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_group_syswide

rdpmc_group_syswide.o: rdpmc_group_syswide.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_group_syswide.c

rdpmc_multiattach: rdpmc_multiattach.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_multiattach.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_multiattach

rdpmc_multiattach.o: rdpmc_multiattach.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_multiattach.c

rdpmc_pthreads: rdpmc_pthreads.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_pthreads.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_pthreads

rdpmc_pthreads.o: rdpmc_pthreads.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_pthreads.c

rdpmc_support: rdpmc_support.o rdpmc_lib.o $(LIB)/libhelper.a
	$(CC) $(CPPFLAGS) $(RPATH) $(LPATH) rdpmc_lib.o rdpmc_support.o $(LIB)/libhelper.a $(LDLIBS) -lperf -o rdpmc_support

rdpmc_support.o: rdpmc_support.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_support.c

rdpmc_lib.o: rdpmc_lib.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_lib.c

install: all
	$(INSTALL) -d $(prefix)/tests/rdpmc_libperf
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/rdpmc_libperf

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
