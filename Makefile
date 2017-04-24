main: main.o types.o
	gcc main.o types.o -o main

main.o: main.c
	gcc -c main.c

types.o: types.c types.h
	gcc -c types.c

clean:
	rm *.o main


