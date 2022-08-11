__kernel void convolution(__global float *inputImage, __global float *outputImage, __global float *filter, __global int *filterWidth) 
{
    float sum = 0;
    int i, j;
    int index = get_global_id(0);

    int RowID = index / 600;
    int ColID = index % 600;

    int filterCenter = (*filterWidth * *filterWidth -1) / 2;

    for( i=-1 ; i<=1 ; i++)
    {
        for( j=-1 ; j<=1 ; j++)
        {
            sum += inputImage[(RowID+i) * 600 + ColID + j] * filter[filterCenter + *filterWidth*i + j];
        }
    }

    outputImage[index] = sum;
}

