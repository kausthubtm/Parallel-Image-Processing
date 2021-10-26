#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> 
#include "../../lodepng.h"

int z[1000000000]

int main(int argc,char *argv[ ])
{
    int size,rank,x[100000000],y[100000000],i;
    int len;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank==0)
    {
        printf("Hello from process %d\n", rank);
        for(i=0;i<8;i++)
            x[i] = i;

        printf("%d", x[3]);
    }

    len = 2;
    // printf("\nHello from process : %d\n", rank);
    MPI_Scatter(x, 2, MPI_INT, y, 2, MPI_INT, 0, MPI_COMM_WORLD);

    for(int i=0; i<len; i++)
    {
        // printf("%d ", y[i]);
        y[i] = y[i]+100;
    }
    // printf("\n");

    MPI_Gather(y, 2, MPI_INT, z, 2, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Gather done with process : %d\n", rank);
    if(rank==0){
        for(int i=0; i<8; i++){
            printf("%d ",z[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}