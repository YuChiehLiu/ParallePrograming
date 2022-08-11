#include <stdio.h>
#include <stdlib.h>
#include "hostFE.h"
#include "helper.h"

void hostFE(int filterWidth, float *filter, int imageHeight, int imageWidth,
            float *inputImage, float *outputImage, cl_device_id *device,
            cl_context *context, cl_program *program)
{
    cl_int status;
    int filterSize = filterWidth * filterWidth * sizeof(float);
    int imageSize = imageHeight * imageWidth * sizeof(float);

    cl_command_queue myqueue;
    myqueue = clCreateCommandQueue(*context, *device, 0, &status);

    cl_mem img_ip = clCreateBuffer(*context, CL_MEM_READ_ONLY, imageSize, NULL, &status);
    cl_mem img_op = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, imageSize, NULL, &status);
    cl_mem flt_ip = clCreateBuffer(*context, CL_MEM_READ_ONLY, filterSize, NULL, &status);
    cl_mem flt_width = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(int), NULL, &status);
    
    status = clEnqueueWriteBuffer(myqueue, img_ip, CL_TRUE, 0, imageSize, (void*)inputImage, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(myqueue, flt_ip, CL_TRUE, 0, filterSize, (void*)filter, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(myqueue, flt_width, CL_TRUE, 0, sizeof(int), (void*)&filterWidth, 0, NULL, NULL);
    

    cl_kernel kernel;
    kernel = clCreateKernel(*program, "convolution", 0);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &img_ip);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &img_op);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &flt_ip);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &flt_width);

    // Height = 400
    // Width  = 600

    size_t global_work_size = imageHeight*imageWidth;
    //size_t local_work_size = imageWidth;

    clEnqueueNDRangeKernel(myqueue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

    clFinish(myqueue);

    clEnqueueReadBuffer(myqueue, img_op, CL_TRUE, 0, imageSize, (void*)outputImage, 0, NULL, NULL);

    clReleaseKernel(kernel);
    clReleaseMemObject(img_ip);
    clReleaseMemObject(img_op);
    clReleaseMemObject(flt_ip);
    clReleaseCommandQueue(myqueue);

}