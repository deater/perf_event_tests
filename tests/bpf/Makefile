include ../../Makefile.globals.inc

BPF_CC = clang
#CC = clang-3.7
#CFLAGS = -O2 -emit-llvm
BPF_CFLAGS = -Wno-unused-value -Wno-pointer-sign -O2 -target bpf -I../../include

CPPFLAGS +=-I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	load_bpf

all:	$(PROGRAM_LIST) test_bpf.o test_bpf_output.o

load_bpf:	load_bpf.o $(LIB)/libhelper.a

test_bpf.o:	test_bpf.c
	$(BPF_CC) $(BPF_CFLAGS) -c test_bpf.c -o test_bpf.o

test_bpf_output.o:	test_bpf_output.c
	$(BPF_CC) $(BPF_CFLAGS) -c test_bpf_output.c -o test_bpf_output.o


clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~

