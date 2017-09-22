output: main.o box.o
	gcc main.o box.o -o program

main.o: main.c
	gcc -c main.c

box.o: box.c box.h
	gcc -c box.c

clean:
	rm *.o program
