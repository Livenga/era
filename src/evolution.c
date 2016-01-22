#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "../include/opencl.h"
#define __MAIN_EVOLUTION__
#include "../include/genetic.h"
#include "../include/canvas.h"
#include "../include/common.h"
#include "../include/evolution.h"

static void
sum_data(size_t num_datas,
         size_t width,
         size_t height,
         double *ncv_data,
         double *dest) {
  const size_t nsize = CanvasSize(width, height);
  int i, j, x, y;

  for(i = 0; i < num_datas; i++) {
    for(j = 0; j < NUMBER_OF_GABOR; j++) {
      for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
          dest[nsize * i + (y * width + x)] +=
            ncv_data[nsize * i * j + (y * width + x)];
        }
      }
    }
  }
}

void
evolution(cl_prop        prop,
          const size_t   *num_datas,
          const n_canvas **ncv_data) {
  const size_t nsize =
    CanvasSize(ncv_data[0][0].width, ncv_data[0][0].height);
  const size_t dsize =
    CanvasSize((ncv_data[0][0].width - 4), (ncv_data[0][0].height - 4));

  int i;
  int cnt_chr;

  /* 進化計算用変数 */
  int      cnt_generation;
  genotype pr_gtype[POPULATION_SIZE];

  /* OpenCL 用変数 */
  size_t     feature_size[3];
  cl_feature feature;
  double     *f_in_data, *f_out_data, *f_data;

  /* 入出力結合画像領域の確保 */
  f_in_data  =
    (double *)calloc(nsize * num_datas[0] * NUMBER_OF_GABOR, sizeof(double));
  f_out_data =
    (double *)calloc(dsize * num_datas[0] * NUMBER_OF_GABOR, sizeof(double));
  f_data     =
    (double *)calloc(dsize * num_datas[0], sizeof(double));

  init_cl_feature(&prop, &feature,
      ncv_data[0][0].width, ncv_data[0][0].height,
      num_datas[0]);
  feature_size[0] = ncv_data[0][0].width  - 4;
  feature_size[1] = ncv_data[0][0].height - 4;
  feature_size[2] = num_datas[0] * NUMBER_OF_GABOR;


  char save_path[1024];
  n_canvas n;
  n.width  =  ncv_data[0][0].width  - 4;
  n.height = (ncv_data[0][0].height - 4) * num_datas[0];

  for(i = 0; i < POPULATION_SIZE; i++) {
    pr_gtype[i] = galloc(INTERNAL_SIZE, 0, 1);
    ginit(pr_gtype[i]);

    //gprint(i, pr_gtype[i]);
    for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
      // データの結合
      cat_double(num_datas[cnt_chr] * NUMBER_OF_GABOR, ncv_data[cnt_chr], f_in_data);
      clEnqueueWriteBuffer(prop.queue, feature.cl_gtype, CL_TRUE,
          0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)pr_gtype[i],
          0, NULL, NULL);
      clEnqueueWriteBuffer(prop.queue, feature.cl_ncv_data, CL_TRUE,
          0, sizeof(double) * nsize * num_datas[cnt_chr] * NUMBER_OF_GABOR,
          (const void *)f_in_data, 0, NULL, NULL);
      // 局所受容野
      clEnqueueNDRangeKernel(prop.queue, prop.feature, 3,
          NULL, (const size_t *)feature_size, NULL,
          0, NULL, NULL);
      // 出力の読み込み
      clEnqueueReadBuffer(prop.queue, feature.cl_dest, CL_TRUE,
          0, sizeof(double) * dsize * num_datas[cnt_chr] * NUMBER_OF_GABOR,
          (void *)f_out_data, 0, NULL, NULL);

      // 出力データの加算 */
      bzero((void *)f_data, sizeof(double) * dsize * num_datas[0]);
      sum_data(num_datas[cnt_chr],
          ncv_data[0][0].width - 4, ncv_data[0][0].height - 4,
          f_out_data, f_data);
      n.n = f_data;
      sprintf(save_path, "./WorkSpace/convolution/sum_%c_%04d.png", 'A' + cnt_chr, i);
      pnwrite_from_ncv(save_path, n);
    }
  }


  cnt_generation = 0;
  do {
  } while(++cnt_generation != NUMBER_OF_GENERATION);

  printf("%s[Last Generation] %d%s\n", WHT_STR, cnt_generation, CLR_STR);


  /* 後処理 */
  bzero((void *)f_in_data,  sizeof(double) * num_datas[0] * NUMBER_OF_GABOR * nsize);
  bzero((void *)f_out_data, sizeof(double) * num_datas[0] * NUMBER_OF_GABOR * dsize);
  free(f_out_data); f_out_data = NULL;
  free(f_in_data);  f_in_data  = NULL;

  genotype_destroy(POPULATION_SIZE, pr_gtype);
  cl_feature_destroy(&feature);
}

static void
cat_double(size_t num_datas,
           const  n_canvas *ncv_data,
           double *dest) {
  const size_t nsize =
    CanvasSize(ncv_data[0].width, ncv_data[0].height);

  int i;
  for(i = 0; i < num_datas; i++)
    memmove((void *)(dest + (i * nsize)),
        (const void *)ncv_data[i].n,
        nsize * sizeof(double));
}


static cl_int
init_cl_feature(cl_prop    *prop,
                cl_feature *feature,
                size_t     width,
                size_t     height,
                size_t     num_datas) {
  const size_t nsize = CanvasSize(width, height);
  const size_t dsize = CanvasSize((width - 4), (height - 4));

  cl_int status;

  feature->cl_gtype    = clCreateBuffer(prop->context, CL_MEM_READ_ONLY,
      sizeof(genotype_t) * MAX_GTYPE_SIZE, (void *)NULL, &status);
  feature->cl_ncv_data = clCreateBuffer(prop->context, CL_MEM_READ_ONLY,
      sizeof(double) * nsize * num_datas * NUMBER_OF_GABOR, (void *)NULL, &status);
  feature->cl_dest     = clCreateBuffer(prop->context, CL_MEM_WRITE_ONLY,
      sizeof(double) * dsize * num_datas * NUMBER_OF_GABOR, (void *)NULL, &status);

  status = clSetKernelArg(prop->feature, 0, sizeof(size_t), (void *)&width);
  status = clSetKernelArg(prop->feature, 1, sizeof(size_t), (void *)&height);
  status = clSetKernelArg(prop->feature, 2, sizeof(cl_mem), (void *)&feature->cl_gtype);
  status = clSetKernelArg(prop->feature, 3, sizeof(cl_mem), (void *)&feature->cl_ncv_data);
  status = clSetKernelArg(prop->feature, 4, sizeof(cl_mem), (void *)&feature->cl_dest);

  return status;
}
static void
cl_feature_destroy(cl_feature *feature) {
  clReleaseMemObject(feature->cl_dest);
  clReleaseMemObject(feature->cl_ncv_data);
  clReleaseMemObject(feature->cl_gtype);
}
