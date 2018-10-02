all: floristApp

floristApp: 151044012_main.o 
	gcc 151044012_main.o -o floristApp -pthread -lm

151044012_main.o: 151044012_main.c
	gcc -c 151044012_main.c -pthread -lm

clean:
	rm *.o floristApp
