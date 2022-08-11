#include <iostream>
#include <stdio.h>
#include "CycleTimer.h"

using namespace std;

#define node 10000
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

__global__ void find_min_root(int *min_s, int *s, int k){

    for(int index=0; index<(key-k-1); index++){
        int i = index+1;
        int temp = INT_MAX;
        for(int root = 0; root<(k+1); root++){
            int pos = (k==-1) ? 1 : (k+1);
            //printf("index %d of k = %d : %d \n",i ,k, min_s[index * pos + root]);
            if(min_s[index * pos + root] <= temp){
                 temp = min_s[index * pos + root];
            }
        }

        s[i * key + i+k] = temp;
        //printf("\n");
    }
    //printf("\n");

}

__global__ void compute_s (int *w, int *q,int *s, int k, int *min_s){

    //k = -1 to key-2
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if(k == -1){
        if(tid >= key-k-1)
            return;
        else{
            s[(tid+1) * key + tid] = q[tid];
            //min_s[tid] = q[tid];
            return;
        }
    }
    else if(tid >= (key-k-1)*(k+1)){
        return;
    }
    else{
        int i = tid / (k+1) + 1;
        int r = i + tid % (k+1);

        min_s[tid] = s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k];

        //if(r == i){
        //    s[i*key+i+k] = INT_MAX;
        //}
        //__syncthreads();
        //int min = s[i*key+r-1] + s[(r+1)*key+i+k] +  w[i*key+i+k];
        //
        ////lock
        //while(0 == atomicCAS(&mlock, 0, 1)){
        //    //critical section
        //    if(min < s[i*key+i+k]){
        //        s[i*key+i+k] = min;
        //    }

        //    //unlock
        //    atomicExch(&mlock, 0);
        //}

    }
    return;
}

int main(){
    //warm up gpu
    int grid_sizeW = ceil((double)(key) * ((double)(key) + 1) / 2.0 / (double)block_size);
    warm_up_gpu<<<grid_sizeW, block_size>>>();
    
    double start_time, end_time;

    //int w[key+1][key] = {0};
    //int s[key+1][key] = {0};
    int *w = (int *)malloc(sizeof(int) * (key+1) * key);
    int *s = (int *)malloc(sizeof(int) * (key+1) * key);
    int *p = (int*)malloc(key * sizeof(int));
    int *q = (int*)malloc(key * sizeof(int));
    generate_data(p, q);

    start_time = currentSeconds();
    int *w_d;
    cudaMalloc(&w_d, sizeof(int) * (key+1) * (key));
    int *p_d, *q_d;
    cudaMalloc(&p_d, sizeof(int) * (key));
    cudaMalloc(&q_d, sizeof(int) * (key));
    cudaMemcpy(p_d, p, sizeof(int) * (key), cudaMemcpyDefault);
    cudaMemcpy(q_d, q, sizeof(int) * (key), cudaMemcpyDefault);

    compute_w<<<grid_sizeW, block_size>>>(w_d, p_d, q_d);
    cudaDeviceSynchronize();
    cudaMemcpy(w, w_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault );

    int *s_d;
    cudaMalloc(&s_d, sizeof(int) * (key+1) * (key));
    cudaMemcpy(s_d, s, sizeof(int) * (key+1) * (key), cudaMemcpyDefault);

    for(int k=-1; k<=key-2 ; k++){
        int grid_sizeS = ceil( double((key-k-1) * (k+1)) / block_size);
        if(k == -1)
            grid_sizeS =  ceil( double(key-k-1) / block_size);

        int min_size = (k==-1) ? (key-k-1) : (key-k-1) * (k+1);

        int *min_s = (int *)malloc(sizeof(int) * min_size);
        memset(min_s, INT_MAX, min_size);
        int *min_s_d;
        cudaMalloc(&min_s_d, sizeof(int) * min_size);
        cudaMemcpy(min_s_d, min_s, sizeof(int) * min_size, cudaMemcpyDefault);

        compute_s<<< grid_sizeS , block_size  >>>(w_d, q_d, s_d, k, min_s_d);
        cudaDeviceSynchronize();
       
        if(k > -1){
            find_min_root<<<1, 1>>>(min_s_d, s_d, k);
        }

        cudaDeviceSynchronize();

        free(min_s);
        cudaFree(min_s_d);
    }

    cudaMemcpy(s, s_d, sizeof(int) * (key+1) * (key),cudaMemcpyDefault);
    end_time = currentSeconds();
    
    //for(int i=1; i<=key; i++){
    //    for(int j=0; j<=key-1; j++){
    //        cout << w[i][j] << " ";
    //    }
    //    cout << endl;
    //}
    //
    //cout << endl;

    //for(int i=1; i<=key; i++){
    //    for(int j=0; j<=key-1; j++){
    //        cout << s[i][j] << " ";
    //    }
    //    cout << endl;
    //}

    cout << s[1 * key + node] << endl;

    double total_time = end_time - start_time;

    printf("\n[One kernel]:\t\t[%.3f] ms\n\n", total_time * 1000);

    free(s);
    free(w);
    free(p);
    free(q);
    cudaFree(p_d);
    cudaFree(q_d);
    cudaFree(s_d);
    cudaFree(w_d);
}
