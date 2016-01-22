#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "../include/opencl.h"
#include "../include/canvas.h"
#include "../include/common.h"

void
do_pooling(cl_prop  prop,
           size_t   num_datas,
           n_canvas *ncv,
           n_canvas *n_dest) {
  const size_t ncv_size =
    CanvasSize(ncv[0].width, ncv[0].height);
  const size_t dest_size =
    CanvasSize((ncv[0].width / 2), (ncv[0].height / 2));

  int i;
  double *cat_ncv;

  size_t global_size[3];
  cl_int cl_status;
  cl_mem cl_ncv, cl_dest;

  cat_ncv = cat_data(num_datas * NUMBER_OF_GABOR, ncv);

  global_size[0] = ncv[0].width  / 2;
  global_size[1] = ncv[0].height / 2;
  global_size[2] = num_datas * NUMBER_OF_GABOR;

  cl_ncv  = clCreateBuffer(prop.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(double) * ncv_size * num_datas * NUMBER_OF_GABOR,
      (void *)cat_ncv, &cl_status);
  cl_dest = clCreateBuffer(prop.context, CL_MEM_WRITE_ONLY,
      sizeof(double) * dest_size * num_datas * NUMBER_OF_GABOR,
      (void *)NULL, &cl_status);

  clSetKernelArg(prop.pooling, 0, sizeof(size_t), (void *)&ncv[0].width);
  clSetKernelArg(prop.pooling, 1, sizeof(size_t), (void *)&ncv[0].height);
  clSetKernelArg(prop.pooling, 2, sizeof(cl_mem), (void *)&cl_ncv);
  clSetKernelArg(prop.pooling, 3, sizeof(cl_mem), (void *)&cl_dest);

  clEnqueueNDRangeKernel(prop.queue, prop.pooling, 3,
      NULL, (const size_t *)global_size, NULL,
      0, NULL, NULL);

  for(i = 0; i < num_datas * NUMBER_OF_GABOR; i++) {
    clEnqueueReadBuffer(prop.queue, cl_dest, CL_TRUE,
        i * dest_size * sizeof(double), dest_size * sizeof(double),
        (void *)n_dest[i].n, 0, NULL, NULL);
  }

  clReleaseMemObject(cl_dest);
  clReleaseMemObject(cl_ncv);

  bzero((void *)cat_ncv, sizeof(double) * num_datas * NUMBER_OF_GABOR * ncv_size);
  free(cat_ncv); cat_ncv = NULL;
}
