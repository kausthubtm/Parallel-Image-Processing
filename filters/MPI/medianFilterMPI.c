#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 
#include <omp.h>
#include <math.h>
#include "../../lodepng.h"
 
void swap(double* xp, double* yp)
{
    double temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
void selectionSort(double arr[], int n)
{
    int i, j, min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        swap(&arr[min_idx], &arr[i]);
    }
}

unsigned char output_image[512*512*4];
unsigned char cal[512*512*4];
unsigned char rgb_image[550][550][3];

unsigned char find_avg(int y, int x, int color) {

    double pixels[49];

    int YlowerLimit = y-3;
    int YupperLimit = y+3;
    int XlowerLimit = x-3;
    int XupperLimit = x+3;
    int count = 0;

    for(int i=YlowerLimit; i<=YupperLimit; i++){
        for(int j=XlowerLimit; j<=XupperLimit; j++){
            pixels[count] = rgb_image[i][j][color];
            count++;
        }
    }
    selectionSort(pixels, 49);

    return pixels[49/2];

}

int main(int argc,char *argv[ ])
{
    int size,rank;

    const char* filename = argc > 1 ? argv[1] : "../../lena.png";
    
    unsigned error;
    unsigned char* in_image;
    
    unsigned width, height;
    double t1=0, t2;
    int w1, h1;
    int len;

    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank==0) 
    {  
        error = lodepng_decode32_file(&in_image, &width, &height, filename);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
        len = (int)(height*width*4)/size;
        w1 = (int)width;
        h1 = (int)height;

        // forming the padded image and rgb channels
        for(int i=0; i<(int)height; i++) {
            for(int j=0; j<(int) width; j++){
                if(i<=2 || i>=(int) height+3 || j<=2 || j>=(int)width+3){
                  rgb_image[i][j][0] = 0;
                  rgb_image[i][j][1] = 0;
                  rgb_image[i][j][2] = 0;
                }
                else {
                  rgb_image[i][j][0] = in_image[i*width*4 + j*4 + 0];
                  rgb_image[i][j][1] = in_image[i*width*4 + j*4 + 1];
                  rgb_image[i][j][2] = in_image[i*width*4 + j*4 + 2];
                }
                
            }
        }

        t1 = omp_get_wtime();
    }

    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&h1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&w1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rgb_image, 550*550*3, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(output_image, len*size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Scatter(in_image, len, MPI_UNSIGNED_CHAR, cal, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);  
    MPI_Barrier(MPI_COMM_WORLD);

    int iPad = len*rank;
    int jPad = len*rank;
    for(int k=0; k<50; k++){
        for(int i=0; i<len; i++){
            if(i%4==3) cal[i] = 255;
            else cal[i] = find_avg((i+iPad)/(w1*4)+3, ((i+jPad)/4+3)%w1, i%4);
        }
    }
    
    MPI_Gather(cal, len, MPI_UNSIGNED_CHAR, output_image, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if(rank==0){
        t2 = omp_get_wtime();
        printf("%f\n",t2-t1);
        error = lodepng_encode32_file("../../output.png", output_image, width, height);
        if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
        // printf("Done\n");
    }

    MPI_Finalize();
    return 0;
}