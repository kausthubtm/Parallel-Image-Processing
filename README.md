# Parallel-Image-Processing
Parallel implementation (OpenMP and MPI) and its serial implementation of Basic Image Processing algorithms. The repo includes implementations of, RGB to BW, Brighten, Prewitt, Box, Median, Gaussian

## Running a program

### Serial Implementation
- `g++ <your_program.cpp> ../../lodepng.cpp -Wall -Wextra -pedantic -ansi -O3 -fopemp`
- `./a.out`

### OpenMP Implementation
- `g++ <your_program.cpp> ../../lodepng.cpp -Wall -Wextra -pedantic -ansi -O3 -fopenmp`
- `./a.out`

### MPI Implementation 
- `mpicc <my_program.c> ../../lodepng.c -Wall -Wextra -pedantic -ansi -O3 -std=c11 -fopenmp`
- `mpiexec -n <no_of_processors> ./a.out`

## Acknowledgment
- [lodeng](https://github.com/lvandeve/lodepng) : for reading and writing PNG images. 

