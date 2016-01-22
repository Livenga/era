#ifndef __EVOLUTION_H
#define __EVOLUTION_H

#ifdef __MAIN_EVOLUTION__
#ifndef __OPENCL_CL_H
#include <CL/cl.h>
#endif

typedef struct _cl_feature {
  cl_mem cl_gtype;
  cl_mem cl_ncv_data;
  cl_mem cl_dest;
} cl_feature;


/* src/evolution.c */
static void
cat_double(size_t num_datas,
           const  n_canvas *ncv_data,
           double *dest);
static cl_int
init_cl_feature(cl_prop    *prop,
                cl_feature *feature,
                size_t     width,
                size_t     height,
                size_t     num_datas);
static void
cl_feature_destroy(cl_feature *feature);
#endif

#endif
