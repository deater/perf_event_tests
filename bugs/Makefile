CC = gcc
CFLAGS = -Wall -O2
LFLAGS =

all:	sampled_notleader_refresh overflow_requires_mmap

###

overflow_requires_mmap:	overflow_requires_mmap.o
	$(CC) $(LFLAGS) -o overflow_requires_mmap overflow_requires_mmap.o

overflow_requires_mmap.o:	overflow_requires_mmap.c
	$(CC) $(CFLAGS) -c overflow_requires_mmap.c

###

sampled_notleader_refresh:	sampled_notleader_refresh.o
	$(CC) $(LFLAGS) -o sampled_notleader_refresh sampled_notleader_refresh.o

sampled_notleader_refresh.o:	sampled_notleader_refresh.c
	$(CC) $(CFLAGS) -c sampled_notleader_refresh.c

###

clean:	
	rm -f *~ *.o sampled_notleader_refresh overflow_requires_mmap


