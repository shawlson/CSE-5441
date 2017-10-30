all: disposable persistent

disposable: shawlson_dan_disposable.c box.o
	gcc -fopenmp -O3 shawlson_dan_disposable.c box.o -o disposable

persistent: shawlson_dan_persistent.c box.o
	gcc -fopenmp -O3 shawlson_dan_persistent.c box.o -o persistent

box.o: box.c box.h
	gcc -O3 -c box.c

clean:
	rm *.o disposable persistent
