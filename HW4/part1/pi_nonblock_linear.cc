#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

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

    // TODO: MPI init

    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank);

    tosses_per_node = tosses / world_size;

    unsigned int seed = time(NULL) + world_rank * world_rank;

    double x, y, distance_squared;
        count_per_node = 0;

    for( long long int toss=0 ; toss<tosses_per_node ; toss++ )
    {
        x = (double) rand_r(&seed) / RAND_MAX;
        y = (double) rand_r(&seed) / RAND_MAX;
        distance_squared = x * x + y * y;
        if ( distance_squared <= 1)
            count_per_node++;
    }

    if (world_rank > 0)
    {    
        // TODO: MPI workers
        dest=0;       
        MPI_Send(&count_per_node, 1, MPI_LONG_LONG_INT, dest, tag, MPI_COMM_WORLD);
    }
    else if (world_rank == 0)
    {
        // TODO: non-blocking MPI communication.
        // Use MPI_Irecv, MPI_Wait or MPI_Waitall.
        MPI_Request requests[world_size-1];
        long long int count_node_array[world_size-1];

        totalcounts += count_per_node;

        for( source=1 ; source<world_size ; source++ )
        {
            MPI_Irecv(&count_node_array[source-1], 1, MPI_LONG_LONG_INT, source, tag, MPI_COMM_WORLD, &requests[source-1]);
        }

        MPI_Waitall(world_size-1,requests,MPI_STATUS_IGNORE);

        for (int i = 1; i < world_size; i++)
        {
           totalcounts += count_node_array[i-1];
        }              
    }

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
