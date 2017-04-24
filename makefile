filetest: main.o types.o
	gcc -o filetest main.o types.o

main.o: main.c 
	gcc -c main.c -o main.o

types.o: types.c
	gcc -c types.c -o types.o