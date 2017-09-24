output: main.o box.o
	gcc -O3 main.o box.o -o program

main.o: main.c
	gcc -O3 -c main.c

box.o: box.c box.h
	gcc -O3 -c box.c

clean:
	rm *.o program
