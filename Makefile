all: hw2.c
	mpicc hw2.c -o hw2.o -lm
run1:
	mpiexec -n 1 ./hw2.o 16
run2:
	mpiexec -n 2 ./hw2.o 16
run4:
	mpiexec -n 4 ./hw2.o 16
run8:
	mpiexec -n 8 ./hw2.o 16
run16:
	mpiexec -n 16 ./hw2.o 16
