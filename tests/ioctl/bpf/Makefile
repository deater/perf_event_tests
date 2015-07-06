CC = clang-3.7
#CFLAGS = -O2 -emit-llvm
CFLAGS = -Wno-unused-value -Wno-pointer-sign -O2 -target bpf

all:	test_bpf.o

test_bpf.o:	test_bpf.c
	$(CC) $(CFLAGS) -c test_bpf.c -o test_bpf.o

clean:	
	rm -f *~ *.o
