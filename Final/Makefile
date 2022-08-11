all: OBST_improve2 OBST_serial
OBST_improve2: OBST_improve.cu
	nvcc OBST_improve.cu -arch=sm_35 -rdc=true -l cudadevrt -o OBST_improve
	./OBST_improve

OBST_serial: OBST_serial.cu
	nvcc OBST_serial.cu -arch=sm_35 -rdc=true -l cudadevrt -o OBST_serial
#./OBST_serial

clean:
	rm OBST_serial OBST_improve
