#include <iostream>
#include <stdio.h>
#include "CycleTimer.h"

#define node 150 
#define key (node+1)
#define block_size 32.0
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



__global__ void compute_s (int *w, int *q,int *s, int k){
    
    //k = -1 to key-2
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if(tid >= key-k-1)
        return;
    int i = tid+1;
    if(k == -1){
        s[i*(key)+i+k] = q[i+k];
    }
    else{
        int r = i;
        s[i*key+i+k]  = s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k];
        for(r=i; r<=i+k;r++){
            if( s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k] < s[i*key + i+k])
                s[i*key+i+k] = s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k];
        }
    }
    //printf("\n\n");
    return;
}

int main(){
    //warm up gpu
    int grid_size_W = ceil((double)(key) * ((double)(key) + 1) / 2.0 / (double)block_size);
    warm_up_gpu<<<grid_size_W, block_size>>>();
    
    double start_time, end_time;

    int *w, *s;
    w = (int*)calloc((node+2) * (node+1), sizeof(int));
    s = (int*)calloc((node+2) * (node+1), sizeof(int));

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


    compute_w<<<grid_size_W, block_size>>>(w_d, p_d, q_d);
    cudaDeviceSynchronize();

    cudaMemcpy(w, w_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault );

    int *s_d;
    cudaMalloc(&s_d, sizeof(int) * (key+1) * (key));
    

    for(int k=-1; k<=key-2 ; k++){
        //compute_s<<< 1,key-k-1 >>>(w_d, q_d, s_d, k);
        int grid_size_S = ceil((double)(key-k-1) / block_size );
        compute_s<<< grid_size_S, block_size >>>(w_d, q_d, s_d, k);
        cudaDeviceSynchronize();
    }


    cudaMemcpy(s, s_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault);
    end_time = currentSeconds();
    
    /*for(int i=1; i<=key; i++){
        for(int j=0; j<=key-1; j++){
            cout << s[i][j] << " ";
        }
        cout << endl;
    }*/

    cout << s[1*(node+1)+node] << endl;

    

    double total_time = end_time - start_time;

    printf("\n[eric]:\t\t[%.3f] ms\n\n", total_time * 1000);
}
