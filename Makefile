all:
	cd lib && make	
	cd bugs && make
	cd corner_cases && make
	cd crashes && make
	cd generalized_events && make
	cd overflow && make
	cd record_sample && make
	cd validation && make
	cd validation_privileged && make
	cd fuzzer && make
	cd tools && make

papi:
	cd papi && make

clean:	
	rm -rf *~
	cd lib && make clean
	cd bugs && make clean
	cd corner_cases && make clean
	cd crashes && make clean
	cd generalized_events && make clean
	cd overflow && make clean
	cd record_sample && make clean
	cd validation && make clean
	cd validation_privileged && make clean
	cd papi && make clean
	cd include && make clean
	cd fuzzer && make clean
	cd tools && make clean
