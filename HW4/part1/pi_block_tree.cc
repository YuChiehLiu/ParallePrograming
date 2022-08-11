#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char **argv)
{
    int source;
    int dest;
    int tag=0;
    MPI_Status status;
    long long int count_per_node;
    long long int tosses_per_node;
    long long int totalcounts = 0;
    
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---

    // TODO: init MPI
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank);

    double x, y, distance_squared;
    int MAX_EXP=0, size_temp;

    count_per_node = 0;
    tosses_per_node = tosses / world_size;
    size_temp=world_size;
    
    while(size_temp != 1)
    {
        size_temp /= 2;
        MAX_EXP++;
    }
     

    unsigned int seed = time(NULL) + world_rank * world_rank;

    for( long long int toss=0 ; toss<tosses_per_node ; toss++ )
    {
        x = (double) rand_r(&seed) / RAND_MAX;
        y = (double) rand_r(&seed) / RAND_MAX;
        distance_squared = x * x + y * y;
        if ( distance_squared <= 1)
            count_per_node++;
    }

    totalcounts = count_per_node;
    
    for( int i=1 ; i<=MAX_EXP ; i++)
    {
        if( world_rank%((int)pow(2,i)) != 0 && world_rank%((int)pow(2,i-1)) == 0 )
        {
            MPI_Send(&totalcounts, 1, MPI_LONG_LONG_INT, world_rank-pow(2,i-1), tag, MPI_COMM_WORLD);
            break;
        }
        else if( world_rank%((int)pow(2,i)) == 0 )
        {
            MPI_Recv(&totalcounts, 1, MPI_LONG_LONG_INT, world_rank+pow(2,i-1), tag, MPI_COMM_WORLD, &status);
            count_per_node += totalcounts;
            totalcounts = count_per_node;
        }
    }

    // TODO: binary tree redunction

    if (world_rank == 0)
    {
        // TODO: PI result

        pi_result =4 * (double)totalcounts / tosses;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
