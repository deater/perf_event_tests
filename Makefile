all:
	cd lib && make
	cd tests && make
	cd crashes && make
	cd fuzzer && make
	cd tools && make

papi:
	cd papi && make

clean:	
	rm -rf *~
	cd lib && make clean
	cd tests && make clean
	cd crashes && make clean
	cd papi && make clean
	cd include && make clean
	cd fuzzer && make clean
	cd tools && make clean
