all:
	cd lib && make	
	cd bugs && make
	cd crashes && make
	cd validation && make

clean:	
	rm -rf *~
	cd lib && make clean
	cd bugs && make clean
	cd crashes && make clean
	cd validation && make clean

