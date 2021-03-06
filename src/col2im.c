#include <stdio.h>
#include <math.h>
inline void col2im_add_pixel(float *im, int height, int width, int channels,
                        int row, int col, int channel, int pad, float val)
{
    row -= pad;
    col -= pad;

    if (row < 0 || col < 0 ||
        row >= height || col >= width) return;
    im[col + width*(row + height*channel)] += val;
}
//This one might be too, can't remember.
void col2im_cpu(float* data_col,
         int channels,  int height,  int width,
         int ksize,  int stride, int pad, float* data_im) 
{
    int c,h,w;
    int height_col = (height - ksize) / stride + 1;
    int width_col = (width - ksize) / stride + 1;
    if (pad){
        height_col = 1 + (height-1) / stride;
        width_col = 1 + (width-1) / stride;
        pad = ksize/2;
    }
    int channels_col = channels * ksize * ksize;
    for (c = 0; c < channels_col; ++c) {
        int w_offset = c % ksize;
        int h_offset = (c / ksize) % ksize;
        int c_im = c / ksize / ksize;
        for (h = 0; h < height_col; ++h) {
            for (w = 0; w < width_col; ++w) {
                int im_row = h_offset + h * stride;
                int im_col = w_offset + w * stride;
                int col_index = (c * height_col + h) * width_col + w;
                double val = data_col[col_index];
                col2im_add_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad, val);
            }
        }
    }
}


#ifdef GPU

#include "opencl.h"

cl_kernel get_col2im_kernel()
{
    static int init = 0;
    static cl_kernel im2col_kernel;
    if(!init){
        im2col_kernel = get_kernel("src/col2im.cl", "col2im", 0);
        init = 1;
    }
    return im2col_kernel;
}

void col2im_ongpu(cl_mem data_col,  int offset,
        int channels,  int height,  int width,
        int ksize,  int stride,  int pad, cl_mem data_im)
{
    cl_kernel kernel = get_col2im_kernel();
    cl_command_queue queue = cl.queue;

    cl_uint i = 0;
    cl.error = clSetKernelArg(kernel, i++, sizeof(data_col), (void*) &data_col);
    cl.error = clSetKernelArg(kernel, i++, sizeof(offset), (void*) &offset);
    cl.error = clSetKernelArg(kernel, i++, sizeof(channels), (void*) &channels);
    cl.error = clSetKernelArg(kernel, i++, sizeof(height), (void*) &height);
    cl.error = clSetKernelArg(kernel, i++, sizeof(width), (void*) &width);
    cl.error = clSetKernelArg(kernel, i++, sizeof(ksize), (void*) &ksize);
    cl.error = clSetKernelArg(kernel, i++, sizeof(stride), (void*) &stride);
    cl.error = clSetKernelArg(kernel, i++, sizeof(pad), (void*) &pad);
    cl.error = clSetKernelArg(kernel, i++, sizeof(data_im), (void*) &data_im);
    check_error(cl);

    size_t global_size = channels*height*width;

    cl.error = clEnqueueNDRangeKernel(queue, kernel, 1, 0,
            &global_size, 0, 0, 0, 0);
    check_error(cl);
}

/*
   void col2im_gpu(float *data_col,  int batch,
   int channels,  int height,  int width,
   int ksize,  int stride,  int pad, float *data_im) 
   {
   int height_col = (height - ksize) / stride + 1;
   int width_col = (width - ksize) / stride + 1;
   int channels_col = channels * ksize * ksize;

   size_t size = height_col*width_col*channels_col*batch;
   cl_mem col_gpu = cl_make_array(data_col, size);
   size = channels*height*width*batch;
   cl_mem im_gpu = cl_make_array(data_im, size);

   col2im_ongpu(col_gpu, batch, channels, height, width,
   ksize, stride, pad, im_gpu);

   cl_read_array(im_gpu, data_im, size);
   clReleaseMemObject(col_gpu);
   clReleaseMemObject(im_gpu);
   }
 */

#endif
