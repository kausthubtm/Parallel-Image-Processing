# Parallel-Image-Processing
Parallel implementation (OpenMP and MPI) of Basic Image Processing algorithms

## Running a program

### Serial Implementation
- `g++ <your_program.cpp> ../../lodepng.cpp -Wall -Wextra -pedantic -ansi -O3 `
- `./a.out`

### OpenMP Implementation
- `g++ your_program.cpp lodepng.cpp -Wall -Wextra -pedantic -ansi -O3 -fopenmp`
- `./a.out`

### MPI Implementation 
- `mpicc <my_program.c> ../../lodepng.c -Wall -Wextra -pedantic -ansi -O3 -std=c11`
- `mpiexec -n <no_of_processors> ./a.out`

## Acknowledgment
- [lodeng](https://github.com/lvandeve/lodepng) : for reading and writing PNG images. 

