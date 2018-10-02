all: tiffProcessor

tiffProcessor: 151044012_main.o 
	gcc 151044012_main.o -o tiffProcessor

151044012_main.o: 151044012_main.c
	gcc -c 151044012_main.c

clean:
	rm *.o tiffProcessor
