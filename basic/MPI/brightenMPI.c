#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 
#include "../../lodepng.h"

unsigned char output_image[512*512*4];
unsigned char cal[512*512*4];

int main(int argc,char *argv[ ])
{
    int size,rank,i;

    const char* filename = argc > 1 ? argv[1] : "../../lena.png";
    
    unsigned error;
    unsigned char* in_image;
    
    unsigned width, height;
    int len;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank==0) 
    {  
        error = lodepng_decode32_file(&in_image, &width, &height, filename);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
        len = (int)(height*width*4)/size;
    }
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(in_image, len, MPI_UNSIGNED_CHAR, cal, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    for(i=0; i<len; i++)
    {
        cal[i] = 1.5*cal[i] > 255.0 ? 255.0 : 1.5*cal[i];
    }

    MPI_Gather(cal, len, MPI_UNSIGNED_CHAR, output_image, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if(rank==0){
        error = lodepng_encode32_file("../../output.png", output_image, width, height);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
        // printf("Done\n");
    }

    MPI_Finalize();
    return 0;
}