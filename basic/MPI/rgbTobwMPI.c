#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 
#include <omp.h>
#include "../../lodepng.h"

unsigned char output_image[512*512*4];
unsigned char cal[512*512*4];
unsigned char temp[512*512*4];

int main(int argc,char *argv[ ])
{
    int size,rank,i;

    const char* filename = argc > 1 ? argv[1] : "../../lena.png";
    
    unsigned error;
    unsigned char* in_image;
    
    unsigned width, height;
    int len;
    double t1=0, t2;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank==0) 
    {  
        error = lodepng_decode32_file(&in_image, &width, &height, filename);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
        len = (int)(height*width*4)/size;
        t1 = omp_get_wtime();
    }
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(temp, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Scatter(in_image, len, MPI_UNSIGNED_CHAR, cal, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    for(int k=0; k<1000; k++){
        for(i=0; i<len; i=i+4){
            temp[i+0] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+1] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+2] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+3] = cal[i]; 
        }
    }
    

    MPI_Gather(temp, len, MPI_UNSIGNED_CHAR, output_image, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if(rank==0){
        t2 = omp_get_wtime();
        printf("%f\n",t2-t1);
        error = lodepng_encode32_file("../../output.png", output_image, width, height);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    }

    MPI_Finalize();
    return 0;
}