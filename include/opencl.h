#ifndef __OPENCL_PROPERTIES_H
#define __OPENCL_PROPERTIES_H

#ifndef __OPENCL_CL_H
#include <CL/cl.h>
#endif

typedef struct _kernel_code {
  size_t count;
  char **paths;
  char **codes;
} kernel_code;

typedef struct _cl_properties {
  kernel_code kcode;

  cl_uint        num_platforms;
  cl_platform_id *platforms;

  cl_uint        num_devices;
  cl_device_id   *devices;

  cl_context       context;
  cl_command_queue queue;
  cl_program       program;

  cl_kernel        gabor;
  cl_kernel        pooling;
  cl_kernel        feature;
} cl_prop;


/* src/cl/cl_code.c */
extern int
get_kcode_path(const char  *path,
               kernel_code *kcode);
extern int
read_kcode(kernel_code *kcode);


/* src/cl/cl_device.c */
extern cl_int
get_platforms(cl_prop *prop);
extern cl_int
get_devices(int pid,
            cl_prop *prop);
extern void
print_platform_info(cl_prop prop);
extern void
print_device_info(cl_prop prop);


/* src/cl/cl_kernel.c */
extern cl_int
set_kernel(int did,
           cl_prop *prop);


/* src/cl/cl_util.c */
extern void
cl_prop_destroy(cl_prop *prop);

#endif
