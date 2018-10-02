all: multiprocess_DFT

multiprocess_DFT: 151044012_main.o 
	gcc 151044012_main.o -o multiprocess_DFT -lm

151044012_main.o: 151044012_main.c
	gcc -c 151044012_main.c

clean:
	rm *.o multiprocess_DFT
