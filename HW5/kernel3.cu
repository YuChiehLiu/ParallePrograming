#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>

__device__ int mandeldevice(float c_re, float c_im, int count)
{
  float z_re = c_re, z_im = c_im;
  int i;
  for (i = 0; i < count; ++i)
  {

    if (z_re * z_re + z_im * z_im > 4.f)
      break;

    float new_re = z_re * z_re - z_im * z_im;
    float new_im = 2.f * z_re * z_im;
    z_re = c_re + new_re;
    z_im = c_im + new_im;
  }

  return i;
}

__global__ void mandelKernel(float lowerX, float lowerY, float stepX, float stepY,int *output, int maxIterations, size_t pitch)
{
    // To avoid error caused by the floating number, use the following pseudo code
    //
    int index = blockIdx.x*32*3 + threadIdx.x*3;
    
    for(int i=0 ; i<3 ; i++)
    {
      int thisY = index / 1600;
      int thisX = index % 1600;
      //int thisX = index - thisY*1600;
    
      float x = lowerX + thisX * stepX;
      float y = lowerY + thisY * stepY;

      int *head;

      head = (int*)((char*)output + thisY * pitch);

      //output[index]= mandeldevice(x, y, maxIterations);
      head[thisX] = mandeldevice(x, y, maxIterations);

      index++;
    }
}

//variables correspondence
//upperX -> x1
//upperY -> y1
//lowerX -> x0
//lowerY -> y0
//*img -> output
//resX -> width
//rexY -> height

// Host front-end function that allocates the memory and launches the GPU kernel
void hostFE (float upperX, float upperY, float lowerX, float lowerY, int* img, int resX, int resY, int maxIterations)
{
    float stepX = (upperX - lowerX) / resX;
    float stepY = (upperY - lowerY) / resY;

    int img_size = resX * resY * sizeof(int);

    int *img_host;
    cudaHostAlloc((void**)&img_host, img_size, cudaHostAllocDefault);

    img_host = img;

    int *img_dev;

    size_t pitch;

    cudaMallocPitch((void**)&img_dev, &pitch, resX*sizeof(int), resY);

    mandelKernel<<< 20000,32 >>>( lowerX, lowerY, stepX, stepY, img_dev, maxIterations, pitch);

    cudaMemcpy2D(img_host, resX*sizeof(int), img_dev, pitch, resX*sizeof(int), resY, cudaMemcpyDefault);


    cudaFree(img_dev);
    //cudaFreeHost(img_host);
}
