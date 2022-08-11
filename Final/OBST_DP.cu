#include <iostream>
#include <stdio.h>
#include "CycleTimer.h"

#define node 100
#define key (node+1)
#define block_size 32
using namespace std;
void generate_data(int *p, int *q){
    srand(0);    
   
    for(int i=0; i<=node; i++){
        p[i] = rand() % 10;
        q[i] = rand() % 10;
    }
}
__global__ void warm_up_gpu(){
  unsigned int tid = blockIdx.x * blockDim.x + threadIdx.x;
  float ia, ib;
  ia = ib = 0.0f;
  ib += ia + tid;
}

__global__ void compute_w (int *w, int *p, int *q){
    int tid = blockDim.x * blockIdx.x + threadIdx.x;

    if(tid < (key) * ((key)+1) / 2){
        int temp = tid;
        int index_i = 1;
        for(int i=key; i>0; i--){
            if(tid - i < 0){
                break;
            }

            tid -= i;
            index_i++;
        }
        int index_j = index_i + tid -1 ;

        for(int k = index_i-1; k <= index_j; k++)
            w[index_i*(key) + index_j] += q[k];
        for(int k = index_i; k <= index_j; k++)
            w[index_i*(key) + index_j] += p[k];
    }
    return;
}

__global__ void find_min_root(int i, int k, int *s,  int *w, int *min){
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    
    int r = tid+i;

    //printf("%d \n r:%d \n s[%d][%d] : %d, s[%d][%d] : %d, w[%d][%d] : %d\n",tid , r, i, r-1, s[i*key+r-1], r+1, i+k, s[(r+1)*key+i+k], i, i+k, w[i*key+i+k]);
    min[tid]  = s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k];

}



__global__ void compute_s (int *w, int *q,int *s, int k){
    
    //k = -1 to key-2
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    int i = tid+1;
    if(k == -1){
        s[i*(key)+i+k] = q[i+k];
        //printf("s[%d][%d] = %d \n", i, i+k,s[i*(key)+i+k]);
    }
    else{
        int *min = (int *)malloc(sizeof(int) * (k+1));

        find_min_root<<<1,k+1>>>(i, k, s, w, min);
        
        cudaDeviceSynchronize();

        //printf("\n");
        s[i*key+i+k] = INT_MAX;
        for(int index = 0; index<k+1; index++){
           // printf("%d ",  min[index]);
            if(min[index] <  s[i*key+i+k])
                s[i*key+i+k] = min[index];
        }
        //printf("s[%d][%d] = %d \n", i, i+k, s[i*(key)+i+k]);
    }
    //printf("\n\n");
    return;
}

int main(){
    //warm up gpu
    int grid_size = ceil((double)(key) * ((double)(key) + 1) / 2.0 / (double)block_size);
    warm_up_gpu<<<grid_size, block_size>>>();
    
    double start_time, end_time;

    int w[key+1][key] = {0};
    //int *w = (int*)calloc(key*(key+1) ,sizeof(int));
    int s[key+1][key] = {0};
    //int root[key][key];
    //int p[key] = {0,5,2,4,3};
    //int q[key] = {3,2,3,4,2};
    int *p = (int*)malloc(key * sizeof(int));
    int *q = (int*)malloc(key * sizeof(int));
    generate_data(p, q);

    //serial(p, q);

    start_time = currentSeconds();
    int *w_d;
    cudaMalloc(&w_d, sizeof(int) * (key+1) * (key));
    int *p_d, *q_d;
    cudaMalloc(&p_d, sizeof(int) * (key));
    cudaMalloc(&q_d, sizeof(int) * (key));
    cudaMemcpy(p_d, p, sizeof(int) * (key), cudaMemcpyDefault);
    cudaMemcpy(q_d, q, sizeof(int) * (key), cudaMemcpyDefault);


    compute_w<<<grid_size, block_size>>>(w_d, p_d, q_d);
    cudaDeviceSynchronize();

    cudaMemcpy(w, w_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault );

    int *s_d;
    cudaMalloc(&s_d, sizeof(int) * (key+1) * (key));
    

    for(int k=-1; k<=key-2 ; k++){
        //compute_s<<< 1,key-k-1 >>>(w_d, q_d, s_d, k);
        compute_s<<< key-k-1, 1 >>>(w_d, q_d, s_d, k);
        cudaDeviceSynchronize();
    }

    cudaMemcpy(s, s_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault);
    cudaDeviceSynchronize();

    end_time = currentSeconds();
    
    /*
    for(int i=1; i<=key; i++){
        for(int j=0; j<=key-1; j++){
            cout << s[i][j] << " ";
        }
        cout << endl;
    }
    */

    cout << s[1][key-1] << endl;

    

    double total_time = end_time - start_time;

    printf("\n[Dynamic Parallel]:\t\t[%.3f] ms\n\n", total_time * 1000);
}
