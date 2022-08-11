#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
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

    MPI_Win win;

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

    if (world_rank == 0)
    {
        // Master

        totalcounts += count_per_node;

        long long int *count_arr;
        MPI_Alloc_mem(world_size*sizeof(long long int), MPI_INFO_NULL, &count_arr);

        MPI_Win_create(count_arr, world_size*sizeof(long long int), sizeof(long long int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

        int ready = 0;
        while(!ready)
        {
            MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, win);
            for(int i=1 ; i<world_size ; i++)
            {
                if(count_arr[i] == 0)
                {
                    ready = 0;
                    break;
                }
                else
                {
                    ready = 1;
                }
            }
            MPI_Win_unlock(0, win);
        }

        for(int i=1; i<world_size ; i++)
        {
            totalcounts += count_arr[i];
        }
        MPI_Free_mem(count_arr);
    }
    else
    {
        // Workers

        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
        MPI_Put(&count_per_node, 1, MPI_LONG_LONG_INT, 0, world_rank, 1, MPI_LONG_LONG_INT, win);
        MPI_Win_unlock(0, win);
    }

    MPI_Win_free(&win);

    if (world_rank == 0)
    {
        // TODO: handle PI result

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