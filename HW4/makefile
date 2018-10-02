all: chiefSystem

chiefSystem: 151044012_main.o 
	gcc 151044012_main.o -o chiefSystem -pthread -lrt

151044012_main.o: 151044012_main.c
	gcc -c 151044012_main.c -pthread -lrt

clean:
	rm *.o chiefSystem
