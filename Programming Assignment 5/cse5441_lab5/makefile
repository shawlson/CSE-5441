all: shawlson_dan_lab5.c
	mpicc -O3 -fopenmp bmpReader.o shawlson_dan_lab5.c -o lab5.out

debug:
	mpicc -O3 -fopenmp -DDEBUG bmpReader.o shawlson_dan_lab5.c -o lab5.out

noopenmp:
	mpicc -O3 -fopenmp -DNO_OPENMP bmpReader.o shawlson_dan_lab5.c -o lab5.out

clean:
	rm lab5.out
