#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <math.h>

void construct_matrices(int *n_ptr, int *m_ptr, int *l_ptr, int **a_mat_ptr, int **b_mat_ptr)
{
    int world_rank;
    int world_size;
    int tag=0;
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /*int worksize;

    if((*n_ptr)%world_size == 0)
    {
        worksize = (*n_ptr) / world_size;
    }
    else
    {
        worksize = ((*n_ptr) / world_size) + 1 ;
    }*/
   
    if( world_rank==0 )
    {
        scanf("%d%d%d", n_ptr, m_ptr, l_ptr);

        *a_mat_ptr = (int*)malloc(sizeof(int) * (*n_ptr) * (*m_ptr));
        *b_mat_ptr = (int*)malloc(sizeof(int) * (*m_ptr) * (*l_ptr));

        for( int i=0 ; i<*n_ptr ; i++)
        {
            for( int j=0 ; j<*m_ptr ; j++)
            {
                scanf("%d", (*a_mat_ptr) + i*(*m_ptr) +j);
            }
        }
        for( int i=0 ; i<*m_ptr ; i++)
        {
            for( int j=0 ; j<*l_ptr ; j++)
            {
                scanf("%d", (*b_mat_ptr) + i*(*l_ptr) +j);
            }
        }
        for( int i=1 ; i<world_size ; i++)
        {
            MPI_Send(n_ptr, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(m_ptr, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(l_ptr, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }

        
        int worksize = (*n_ptr) / world_size;

        for( int i=1 ; i<world_size ; i++)
        {
            if( i!=world_size-1)
            {
                MPI_Send((*a_mat_ptr) + i * worksize * (*m_ptr), worksize * (*m_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);
                MPI_Send(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send((*a_mat_ptr) + i * worksize * (*m_ptr), (*n_ptr) * (*m_ptr) - i * worksize * (*m_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);
                MPI_Send(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);

            }
        }
        // MPI_Send((*a_mat_ptr) + (world_size-1) * worksize * (*m_ptr), (*n_ptr) * (*m_ptr) - (world_size-1) * worksize * (*m_ptr), MPI_INT, world_size-1, tag, MPI_COMM_WORLD);
        // MPI_Send(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, world_size-1, tag, MPI_COMM_WORLD);

        /*for( int i=1 ; i<world_size ; i++)
        {
            MPI_Send((*a_mat_ptr) + i * worksize * (*m_ptr), worksize * (*m_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, i, tag, MPI_COMM_WORLD);
        }*/
    }
    else
    {
        MPI_Recv(n_ptr, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(m_ptr, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(l_ptr, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

        *a_mat_ptr = (int*)malloc(sizeof(int) * (*n_ptr) * (*m_ptr));
        *b_mat_ptr = (int*)malloc(sizeof(int) * (*m_ptr) * (*l_ptr));

        int worksize = (*n_ptr) / world_size;

        /*int worksize;

        if((*n_ptr)%world_size == 0)
        {
            worksize = (*n_ptr) / world_size;
        }
        else
        {
            worksize = ((*n_ptr) / world_size) + 1 ;
        }*/

        if( world_rank != world_size-1)
        {
            MPI_Recv(*a_mat_ptr, worksize * (*m_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }
        else
        {
            MPI_Recv(*a_mat_ptr, (*n_ptr) * (*m_ptr) - (world_size-1) * worksize * (*m_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }

        // MPI_Recv(*a_mat_ptr, worksize * (*m_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        // MPI_Recv(*b_mat_ptr, (*m_ptr) * (*l_ptr), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        
    }
}
void matrix_multiply(const int n, const int m, const int l, const int *a_mat, const int *b_mat)
{
    int world_rank;
    int world_size;
    int tag=0;
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int *output=(int*)malloc(sizeof(int) * n * l);
    MPI_Request requests[world_size-1];

    int worksize = n / world_size;

    /*int worksize;

    if(n%world_size == 0)
    {
        worksize = n / world_size;
    }
    else
    {
        worksize = (n / world_size) + 1 ;
    }*/

    if( world_rank != world_size-1)
    {
        for( int i=0 ; i<worksize ; i++)
        {
            for( int j=0 ; j<l ; j++)
            {
                int temp=0;
                for( int k=0 ; k<m ; k++)
                {
                    temp += a_mat[ i*m+k ] * b_mat[ k*l+j ];
                }
                //printf("%d ", temp);
                output[i*l+j] = temp;
            }
            //printf("\n");
        }
    }
    else
    {
        for( int i=0 ; i<n-worksize*(world_size-1) ; i++)
        {
            for( int j=0 ; j<l ; j++)
            {
                int temp=0;
                for( int k=0 ; k<m ; k++)
                {
                    temp += a_mat[ i*m+k ] * b_mat[ k*l+j ];
                }
                //printf("%d ", temp);
                output[i*l+j] = temp;
            }
            //printf("\n");
        }
    }

    
    if((300 <= n) && (n <= 500))
    {
        int MAX_EXP=0, size_temp;

        size_temp=world_size;
        
        while(size_temp != 1)
        {
            size_temp /= 2;
            MAX_EXP++;
        }

        for( int i=1 ; i<=MAX_EXP ; i++)
        {
            if( world_rank%((int)pow(2,i)) != 0 && world_rank%((int)pow(2,i-1)) == 0 )
            {
                if( world_rank != world_size - (int)pow(2,i-1))
                {
                    MPI_Send(output, worksize*l*(int)pow(2,i-1), MPI_INT, world_rank-pow(2,i-1), tag, MPI_COMM_WORLD);
                    break;
                }
                else
                {
                    MPI_Send(output, (n*l) - world_rank*worksize*l, MPI_INT, world_rank-pow(2,i-1), tag, MPI_COMM_WORLD);
                    break;
                }
            }
            else if( world_rank%((int)pow(2,i)) == 0 )
            {
                if( world_rank != world_size - (int)pow(2,i))
                {
                    MPI_Recv(output + worksize*l*(int)pow(2,i-1), worksize*l*(int)pow(2,i-1), MPI_INT, world_rank+pow(2,i-1), tag, MPI_COMM_WORLD, &status);
                }
                else
                {
                    MPI_Recv(output + worksize*l*(int)pow(2,i-1), (n*l) - (world_rank+(int)pow(2,i-1))*worksize*l, MPI_INT, world_rank+pow(2,i-1), tag, MPI_COMM_WORLD, &status);
                }
            }
        }
    }


    else if((1000 <= n) && (n <= 2000))
    {
        if(world_rank == 0)
        {
            for( int i=1 ; i<world_size ; i++)
            {
                if( i!=world_size-1 )
                {
                    MPI_Irecv(output + i*worksize*l, worksize*l, MPI_INT, i, tag, MPI_COMM_WORLD, &requests[i-1]);
                }
                else
                {
                    MPI_Irecv(output + i*worksize*l, (n*l) - i*worksize*l, MPI_INT, i, tag, MPI_COMM_WORLD, &requests[i-1]);   
                }
            }

            MPI_Waitall(world_size-1,requests,MPI_STATUS_IGNORE);
        }
        else
        {
            if( world_rank != world_size-1)
            {
                MPI_Send(output, worksize*l, MPI_INT, 0, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(output, (n*l) - (world_size-1)*worksize*l, MPI_INT, 0, tag, MPI_COMM_WORLD);
            }
        }
    }
    

    else if( n<300 )
    {
        if( world_rank>0 )
        {
            if( world_rank != world_size-1)
            {
                MPI_Send(output, worksize*l, MPI_INT, 0, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(output, (n*l) - (world_size-1)*worksize*l, MPI_INT, 0, tag, MPI_COMM_WORLD);
            }
        }
        else if( world_rank==0 )
        {
            for( int i=1 ; i<world_size ; i++)
            {
                if( i!=world_size-1 )
                {
                    MPI_Recv(output + i*worksize*l, worksize*l, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                }
                else
                {
                    MPI_Recv(output + i*worksize*l, (n*l) - i*worksize*l, MPI_INT, i, tag, MPI_COMM_WORLD, &status);   
                }
            }
            //MPI_Recv(output+(world_size-1)*worksize*l, (n*l) - (world_size-1)*worksize*l, MPI_INT, world_size-1, tag, MPI_COMM_WORLD, &status);
        }
    }


    if( world_rank == 0)
    {
        for( int i=0 ; i<n ; i++)
        {
            for( int j=0 ; j<l ; j++)
            {
                printf("%d ", *(output+i*l+j));
            }
            printf("\n");
        }
    }

    free(output);
}

void destruct_matrices(int *a_mat, int *b_mat)
{
    free(a_mat);
    free(b_mat);
}