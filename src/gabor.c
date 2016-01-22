#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "../include/opencl.h"
#include "../include/canvas.h"
#include "../include/common.h"

void
do_gabor(cl_prop prop,
         n_canvas n_data,
         n_canvas *n_gabor,
         n_canvas *n_dest) {
  const size_t gabor_size =
    CanvasSize(n_gabor[0].width, n_gabor[0].height);
  const size_t ncv_size   =
    CanvasSize(n_data.width,  n_data.height);

  int i;

  double *cat_n_gabor;
  
  size_t global_size[3];
  cl_int cl_status;
  cl_mem cl_gabor, cl_ncv, cl_output;

  /* Gabor フィルタの結合 */
  cat_n_gabor = cat_data(NUMBER_OF_GABOR, n_gabor);


  /* OpenCL 設定 */
  global_size[0] = n_data.width;
  global_size[1] = n_data.height;
  global_size[2] = NUMBER_OF_GABOR;

  cl_gabor  = clCreateBuffer(prop.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(double) * gabor_size * NUMBER_OF_GABOR,
      (void *)cat_n_gabor, &cl_status);
  cl_ncv    = clCreateBuffer(prop.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(double) * ncv_size,
      (void *)n_data.n,  &cl_status);
  cl_output = clCreateBuffer(prop.context, CL_MEM_WRITE_ONLY,
      sizeof(double) * ncv_size * NUMBER_OF_GABOR,
      (void *)NULL, &cl_status);

  clSetKernelArg(prop.gabor, 0, sizeof(size_t), (void *)&n_gabor[0].width);
  clSetKernelArg(prop.gabor, 1, sizeof(size_t), (void *)&n_gabor[0].height);
  clSetKernelArg(prop.gabor, 2, sizeof(cl_mem), (void *)&cl_gabor);
  clSetKernelArg(prop.gabor, 3, sizeof(size_t), (void *)&n_data.width);
  clSetKernelArg(prop.gabor, 4, sizeof(size_t), (void *)&n_data.height);
  clSetKernelArg(prop.gabor, 5, sizeof(cl_mem), (void *)&cl_ncv);
  clSetKernelArg(prop.gabor, 6, sizeof(cl_mem), (void *)&cl_output);


  /* DO */
  clEnqueueNDRangeKernel(prop.queue, prop.gabor, 3,
      NULL, (const size_t *)global_size, NULL,
      0, NULL, NULL);

  for(i = 0; i < NUMBER_OF_GABOR; i++) {
    clEnqueueReadBuffer(prop.queue, cl_output, CL_TRUE,
        i * ncv_size * sizeof(double), ncv_size * sizeof(double),
        (void *)n_dest[i].n, 0, NULL, NULL);
  }


  /* 後処理 */
  clReleaseMemObject(cl_output);
  clReleaseMemObject(cl_ncv);
  clReleaseMemObject(cl_gabor);

  bzero((void *)cat_n_gabor, sizeof(double) * gabor_size * NUMBER_OF_GABOR);
  free(cat_n_gabor); cat_n_gabor = NULL;
}


