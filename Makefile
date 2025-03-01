all:
	@mpicc -Wall -Wextra -fopenmp main.c utils.c -o test

debug:
	@mpicc -DDEBUG -Wall -Wextra -fopenmp main.c utils.c -o test_debug

run_debug:
	@mpiexec -n 3 ./test_debug

run:
	@rm -f performance
	@touch performance
	@mpiexec -n 2 ./test
	@mpiexec -n 3 ./test
	@mpiexec -n 4 ./test
	@mpiexec -n 5 ./test
	@mpiexec -n 6 ./test
	@mpiexec -n 7 ./test
	@mpiexec -n 8 ./test
