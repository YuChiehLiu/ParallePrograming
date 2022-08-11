#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>

__device__ int mandel(float x, float y, int maxIteration){
	float z_re = x;
	float z_im = y;

	int i;
	for(i=0; i<maxIteration; i++){
		if(z_re * z_re + z_im * z_im > 4.f){
			break;
		}

		float new_re = z_re * z_re - z_im * z_im;
		float new_im = 2.f * z_re * z_im;
		z_re = x + new_re;
		z_im = y + new_im;
	}

	return i;
}


__global__ void mandelKernel(float stepX, float stepY, float lowerX, float lowerY, int resX, int resY, int count, int *img_d) {

	int thisX = blockDim.x * blockIdx.x + threadIdx.x;
	int thisY = blockDim.y * blockIdx.y + threadIdx.y;

	if(thisX < resX && thisY < resY){

		//int thisX = tid % resX; //nth in that row
		//int thisY = tid / resX;	//which row

		float x = lowerX + thisX * stepX;
		float y = lowerY + thisY * stepY;

		//float z_re = x, z_im = y;
		//int i;
		//for(i=0; i<count; i++){
		//	if(z_re * z_re + z_im * z_im > 4.f){
		//		break;
		//	}

		//	float new_re = z_re * z_re - z_im * z_im;
		//	float new_im = 2.f * z_re * z_im;
		//	z_re = x + new_re;
		//	z_im = y + new_im;
		//}

		int index = thisY * resX + thisX;
		img_d[index] = mandel(x, y, count);
	}

}

//   Host front-end function that allocates the memory and launches the GPU kerneli
//x1, y1, x0, y0, output, width, height, maxIterations
void hostFE (float upperX, float upperY, float lowerX, float lowerY, int* img, int resX, int resY, int maxIterations)
{
    float stepX = (upperX - lowerX) / resX; //dx
    float stepY = (upperY - lowerY) / resY; //dy

	//int block_size = 256;
	//int num_blocks = resX * resY / 256 + 1;
	dim3 block_size(16,16);
	dim3 grid_size(resX / 16 + 1, resY / 16 + 1);

	int *img_d;
	cudaMalloc(&img_d, resX * resY * sizeof(int));

	mandelKernel<<<grid_size, block_size>>>(stepX, stepY, lowerX, lowerY, resX, resY, maxIterations, img_d);

	cudaMemcpy(img, img_d, resX * resY * sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(img_d);
}
