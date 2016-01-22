#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "../../include/opencl.h"
#include "../../include/common.h"

static void
print_build_log(int did,
                cl_prop *prop);

cl_int
set_kernel(int did,
           cl_prop *prop) {
  cl_int status;

  prop->context = clCreateContext(0, prop->num_devices,
      (const cl_device_id *)prop->devices, NULL, NULL, &status);

  prop->queue = clCreateCommandQueueWithProperties(prop->context,
      prop->devices[did], 0, &status);

  prop->program = clCreateProgramWithSource(prop->context,
      prop->kcode.count, (const char **)prop->kcode.codes, NULL, &status);

  const char *options = "-I./include";
  status = clBuildProgram(prop->program, prop->num_devices,
      (const cl_device_id *)prop->devices, options, NULL, NULL);

  if(status != CL_SUCCESS) {
    printf("%s[Build Error Log]%s\n", ERR_STR, CLR_STR);
  } else {
    printf("%s[Build Log]%s\n", WHT_STR, CLR_STR);
  }
  print_build_log(did, prop);
  if(status != CL_SUCCESS) getchar();

  prop->gabor   =
    clCreateKernel(prop->program, (const char *)"enable_gabor",   NULL);
  prop->pooling =
    clCreateKernel(prop->program, (const char *)"enable_pooling", NULL);
  prop->feature =
    clCreateKernel(prop->program, (const char *)"feature_rfcn",   NULL);

  return status;
}

static void
print_build_log(int did,
                cl_prop *prop) {
  size_t param_size;
  char *param;

  clGetProgramBuildInfo(prop->program,
      prop->devices[did], CL_PROGRAM_BUILD_LOG, 0, NULL, &param_size);

  param = (char *)calloc(param_size, sizeof(char));
  clGetProgramBuildInfo(prop->program,
      prop->devices[did], CL_PROGRAM_BUILD_LOG,
      sizeof(char) * param_size, (void *)param, &param_size);

  printf("%s\n", param);

  memset((void *)param, '\0', sizeof(char) * param_size);
  free(param);
  param = NULL;
}
