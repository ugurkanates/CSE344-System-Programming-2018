all : program1 program2

program1:
	gcc -o homeworkServer thread2.c threadpool.c -pthread -lm

program2:
	gcc -o clientApp client.c

clean:
	rm *.o homeworkServer clientApp
