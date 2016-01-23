#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "../../include/opencl.h"

static void
kernel_code_destroy(kernel_code *kcode);

void
cl_prop_destroy(cl_prop *prop) {
  clReleaseKernel(prop->cls);
  clReleaseKernel(prop->feature);
  clReleaseKernel(prop->pooling);
  clReleaseKernel(prop->gabor);
  clReleaseProgram(prop->program);
  clReleaseCommandQueue(prop->queue);
  clReleaseContext(prop->context);

  kernel_code_destroy(&prop->kcode);
}

static void
kernel_code_destroy(kernel_code *kcode) {
  int i;

  for(i = 0; i < kcode->count; i++) {
    free(kcode->codes[i]); kcode->codes[i] = NULL;
    free(kcode->paths[i]); kcode->paths[i] = NULL;
  }
  free(kcode->codes); kcode->codes = NULL;
  free(kcode->paths); kcode->paths = NULL;
}
