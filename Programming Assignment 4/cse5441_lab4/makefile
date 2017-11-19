NVCC=nvcc
CUDAFLAGS= -O

all: lab4p1 lab4p1_serial lab4p2 lab4p2_serial
debug: CUDAFLAGS += -DDEBUG
debug: lab4p1 lab4p1_serial lab4p2 lab4p2_serial

lab4p1: shawlson_dan_lab4p1.cu
	$(NVCC) $(CUDAFLAGS) shawlson_dan_lab4p1.cu -o lab4p1

lab4p1_serial: shawlson_dan_serial-lab4p1.cu
	$(NVCC) $(CUDAFLAGS) shawlson_dan_serial-lab4p1.cu -o lab4p1_serial

clean:
	rm lab4p1*