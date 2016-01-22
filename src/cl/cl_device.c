#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "../../include/opencl.h"
#include "../../include/common.h"

static cl_int
numof_platforms(cl_prop *prop) {
  return clGetPlatformIDs(0, NULL, &prop->num_platforms);
}

cl_int
get_platforms(cl_prop *prop) {
  cl_int status;

  numof_platforms(prop);
  prop->platforms =
    (cl_platform_id *)calloc(prop->num_platforms, sizeof(cl_platform_id));
  status = clGetPlatformIDs(prop->num_platforms,
      prop->platforms, NULL);

  return status;
}

static cl_int
numof_devices(int pid,
              cl_prop *prop) {
  return clGetDeviceIDs(prop->platforms[pid], CL_DEVICE_TYPE_ALL,
      0, NULL, &prop->num_devices);
}

cl_int
get_devices(int pid,
            cl_prop *prop) {
  cl_int status;

  numof_devices(pid, prop);
  prop->devices =
    (cl_device_id *)calloc(prop->num_devices, sizeof(cl_device_id));
  status = clGetDeviceIDs(prop->platforms[pid], CL_DEVICE_TYPE_ALL,
      prop->num_devices, prop->devices, NULL);

  return status;
}


void
print_platform_info(cl_prop prop) {
  int i;
  char param[4096];

  for(i = 0; i < prop.num_platforms; i++) {
    printf("[Platform Number] %2d\n", i);
    clGetPlatformInfo(prop.platforms[i], CL_PLATFORM_NAME,
        sizeof(param), (void *)param, NULL);
    printf("[Platform Name]    %s\n", param);

    clGetPlatformInfo(prop.platforms[i], CL_PLATFORM_VERSION,
        sizeof(param), (void *)param, NULL);
    printf("[Platform Version] %s\n", param);

    clGetPlatformInfo(prop.platforms[i], CL_PLATFORM_PROFILE,
        sizeof(param), (void *)param, NULL);
    printf("[Platform Profile] %s\n", param);
    
    clGetPlatformInfo(prop.platforms[i], CL_PLATFORM_VENDOR,
        sizeof(param), (void *)param, NULL);
    printf("[Platform Vendor]  %s\n", param);

    putchar('\n');
  }
  putchar('\n');
}


void
print_device_info(cl_prop prop) {
  int i, j;
  cl_ulong ul_param;
  cl_uint  u_param;
  cl_bool  bool_param;
  size_t *work_item;
  char c_param[4096];

  for(i = 0; i < prop.num_devices; i++) {
    printf("[Device Number] %2d\n", i);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_ADDRESS_BITS,
        sizeof(cl_uint), (void *)&u_param, NULL);
    printf("[Device Address Bits]        %u bits\n", u_param);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_AVAILABLE,
        sizeof(cl_bool), (void *)&bool_param, NULL);
    printf("[Device Available]           %s%s%s\n",
        (bool_param == CL_TRUE) ? GRN_STR : ERR_STR,
        (bool_param == CL_TRUE) ? "Enable" : "Disable",
        CLR_STR);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(cl_ulong), (void *)&ul_param, NULL);
    printf("[Device Local Memory Size]   %lu\n", ul_param);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY,
        sizeof(cl_uint), (void *)&u_param, NULL);
    printf("[Device Max Clock Frequency] %.2f [GHz]\n",
        (double)u_param / (1024.0));

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(cl_uint), (void *)&u_param, NULL);
    printf("[Device Compute Units]       %u\n", u_param);


    clGetDeviceInfo(prop.devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
        sizeof(cl_uint), (void *)&u_param, NULL);
    work_item = (size_t *)calloc(u_param, sizeof(size_t));
    clGetDeviceInfo(prop.devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES,
        sizeof(size_t) * u_param, (void *)work_item, NULL);
    for(j = 0; j < u_param; j++)
      printf("[Device Max Work Item Size]  %ld\n", work_item[j]);
    bzero((void *)work_item, sizeof(size_t) * u_param);
    free(work_item);


    clGetDeviceInfo(prop.devices[i], CL_DEVICE_NAME,
        sizeof(c_param), (void *)c_param, NULL);
    printf("[Device Name]                %s\n", c_param);
    bzero((void *)c_param, sizeof(c_param));

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_PROFILE,
        sizeof(c_param), (void *)c_param, NULL);
    printf("[Device Profile]             %s\n", c_param);
    bzero((void *)c_param, sizeof(c_param));

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_VENDOR_ID,
        sizeof(cl_uint), (void *)&u_param, NULL);
    printf("[Device Vendor ID]           %u\n", u_param);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_VENDOR,
        sizeof(c_param), (void *)c_param, NULL);
    printf("[Device Vendor]              %s\n", c_param);

    clGetDeviceInfo(prop.devices[i], CL_DEVICE_VERSION,
        sizeof(c_param), (void *)c_param, NULL);
    printf("[Device Version]             %s\n", c_param);


    putchar('\n');
  }
  putchar('\n');
}
