#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 
#include <omp.h>
#include <math.h>
#include "../../lodepng.h"

unsigned char output_image[512*512*4];
unsigned char bw_image[512*512*4];
unsigned char cal[512*512*4];
unsigned char temp[512*512*4];
unsigned char padded_image[550][550][3];

unsigned char conv(int y, int x, int color, double kernel[3][3]) {

    double ans=0;

    int YlowerLimit = y-1;
    int YupperLimit = y+1;
    int XlowerLimit = x-1;
    int XupperLimit = x+1;

    for(int i=YlowerLimit; i<=YupperLimit; i++){
        for(int j=XlowerLimit; j<=XupperLimit; j++){
            ans = ans + ((int)padded_image[i][j][color])*kernel[i-YlowerLimit][j-XlowerLimit];
        }
    }

    return ans;

}


int main(int argc,char *argv[ ])
{
    int size,rank;

    const char* filename = argc > 1 ? argv[1] : "../../lena.png";
    
    unsigned error;
    unsigned char* in_image;
    
    unsigned width, height;
    int temp2=0, threshold= 170;
    double t1=0, t2;
    int w1, h1;
    int len;

    // kernels 
    double kernel1[3][3] = {{-1,0,1},{-1,0,1},{-1,0,1}};
    double kernel2[3][3] = {{-1,-1,-1},{0,0,0},{1,1,1}};

    
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
        t1 = omp_get_wtime();
    }

    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&h1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&w1, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(temp, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(bw_image, len*size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // bw image
    MPI_Scatter(in_image, len, MPI_UNSIGNED_CHAR, cal, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    for(int k=0; k<1; k++){
        for(int i=0; i<len; i=i+4){
            temp[i+0] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+1] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+2] = cal[i]*0.299 + cal[i+1]*0.587 + cal[i+2]*0.114; 
            temp[i+3] = cal[i]; 
        }
    }
    MPI_Gather(temp, len, MPI_UNSIGNED_CHAR, bw_image, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank==0){
        for(int i=0; i< (int) height; i++) {
            for(int j=0; j< (int) width; j++){
                if(i==0 || i==(int)height+1 || j==2 || j==(int)width+1){
                    padded_image[i][j][0] = 0;
                    padded_image[i][j][1] = 0;
                    padded_image[i][j][2] = 0;
                }
                else {
                    padded_image[i][j][0] = bw_image[i*width*4 + j*4 + 0];
                    padded_image[i][j][1] = bw_image[i*width*4 + j*4 + 1];
                    padded_image[i][j][2] = bw_image[i*width*4 + j*4 + 2];
                }
          
            }
        }
    }

    MPI_Bcast(padded_image, 550*550*3, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(output_image, len*size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&temp2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&threshold, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(kernel1, 9, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(kernel2, 9, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatter(bw_image, len, MPI_UNSIGNED_CHAR, cal, len, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);  

    int iPad = len*rank;
    int jPad = len*rank;
    for(int k=0; k<1; k++){
        for(int i=0; i<len; i=i+4){
            temp2 = sqrt( pow(conv((i+iPad)/(w1*4)+3,((i+jPad)/4+3)%w1,0,kernel1),2) + pow(conv((i+iPad)/(w1*4)+3,((i+jPad)/4+3)%w1,0,kernel2),2) );
            if(temp2 > threshold){
                cal[i+0] = 0;
                cal[i+1] = 0;
                cal[i+2] = 0;
                cal[i+3] = 255;
            }
            else {
                cal[i+0] = 2.0*temp2 > 255.0 ? 255.0 : 2.0*temp2;
                cal[i+1] = 2.0*temp2 > 255.0 ? 255.0 : 2.0*temp2;
                cal[i+2] = 2.0*temp2 > 255.0 ? 255.0 : 2.0*temp2;
                cal[i+3] = 255;
            }
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