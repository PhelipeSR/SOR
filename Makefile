main:
	@echo "\n"
	@echo "\033[0;105mPARA COMPILAR O ORIGINAL:  \033[1;31m$$ make original\033[m"
	@echo "\033[0;105mPARA COMPILAR O MPI:       \033[1;31m$$ make mpi\033[m"
	@echo "\033[0;105mPARA COMPILAR O PROCESSOS: \033[1;31m$$ make processos\033[m"
	@echo "\033[0;105mPARA COMPILAR O OPENMP:    \033[1;31m$$ make openmp\033[m"
	@echo "\033[0;105mPARA COMPILAR TODOS:       \033[1;31m$$ make all\033[m"
	@echo "\n"
	@echo "\033[0;104mPARA EXECUTAR O ORIGINAL   \033[1;31m$$ ./original\033[m"
	@echo "\033[0;104mPARA EXECUTAR O MPI        \033[1;31m$$ mpirun -n <numero de trabalhadores> ./mpi\033[m"
	@echo "\033[0;104mPARA EXECUTAR O PROCESSOS  \033[1;31m$$ ./processos <numero de processos>\033[m"
	@echo "\033[0;104mPARA EXECUTAR O OPENMP     \033[1;31m$$ ./openmp\033[m"
	@echo "\n"

original:
	gcc simpleRT-Original.c function.c -o original -lm
	@echo "\033[0;104mPARA EXECUTAR:\033[m"
	@echo "\033[0;101m./original\033[m"

mpi:
	mpicc simpleRT-MPI.c function.c -o mpi -lm
	@echo "\033[0;104mPARA EXECUTAR:\033[m"
	@echo "\033[0;101mmpirun -n <numero de trabalhadores> ./mpi\033[m"

processos:
	gcc simpleRT-Processos.c function.c -o processos -lm
	@echo "\033[0;104mPARA EXECUTAR:\033[m"
	@echo "\033[0;101m./processos <numero de processos>\033[m"

openmp:
	gcc -fopenmp simpleRT-OpenMP.c function.c -o openmp -lm
	@echo "\033[0;104mPARA EXECUTAR:\033[m"
	@echo "\033[0;101m./openmp\033[m"

all:
	gcc simpleRT-Original.c function.c -o original -lm
	mpicc simpleRT-MPI.c function.c -o mpi -lm
	gcc simpleRT-Processos.c function.c -o processos -lm
	gcc -fopenmp simpleRT-OpenMP.c function.c -o openmp -lm