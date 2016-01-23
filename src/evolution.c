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

void
evolution(cl_prop        prop,
          const size_t   *num_datas,
          const n_canvas **ncv_data) {
  // Gabor 特徴量画像サイズ
  const size_t nsize =
    CanvasSize(ncv_data[0][0].width, ncv_data[0][0].height);
  // RFCN 特徴量画像サイズ
  const size_t feature_width = ncv_data[0][0].width  - 4,
        feature_height       = ncv_data[0][0].height - 4;
  const size_t dsize         =
    CanvasSize(feature_width, feature_height);
  // RFCN 特徴プーリング画像サイズ
  const size_t pool_width = feature_width  / 2,
        pool_height       = feature_height / 2;
  const size_t psize      =
    CanvasSize(pool_width, pool_height);

  int i;
  int cnt_chr;

  /* 進化計算用変数 */
  int        cnt_generation;
  genotype   pr_gtype[POPULATION_SIZE], pr_class[POPULATION_SIZE];
  cls_vector *vector[A2Z];

  /* OpenCL 用変数 */
  size_t     feature_size[3], pooling_size[3], class_size;
  cl_feature feature;
  cl_pooling pooling;
  cl_class   cls;
  double     *f_in_data, *f_out_data, *f_data;
  double     *p_data;

  /* 入出力結合画像領域の確保 */
  f_in_data  =
    (double *)calloc(nsize * num_datas[0] * NUMBER_OF_GABOR, sizeof(double));
  f_out_data =
    (double *)calloc(dsize * num_datas[0] * NUMBER_OF_GABOR, sizeof(double));
  f_data     =
    (double *)calloc(dsize * num_datas[0], sizeof(double));
  p_data     =
    (double *)calloc(psize * num_datas[0], sizeof(double));
  /* ベクトル領域の確保 */
  for(i = 0; i < A2Z; i++)
    vector[i] = (cls_vector *)calloc(num_datas[i], sizeof(cls_vector));

  // OpenCL パラメータの設定
  init_cl_feature(&prop, &feature,
      ncv_data[0][0].width, ncv_data[0][0].height,
      num_datas[0]);
  init_cl_pooling(&prop, &pooling,
      feature_width, feature_height,
      num_datas[0]);
  init_cl_class(&prop, &cls,
      pool_width, pool_height,
      num_datas[0]);
  
  // 並列実行サイズ
  feature_size[0] = feature_width;
  feature_size[1] = feature_height;
  feature_size[2] = num_datas[0] * NUMBER_OF_GABOR;

  pooling_size[0] = pool_width;
  pooling_size[1] = pool_height;
  pooling_size[2] = num_datas[0];

  class_size      = num_datas[0];


  for(i = 0; i < POPULATION_SIZE; i++) {
    pr_gtype[i] = galloc(INTERNAL_SIZE, 0, 1);
    pr_class[i] = galloc(INTERNAL_SIZE, 0, 3);
    ginit(pr_gtype[i]);
    ginit(pr_class[i]);

    for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
      // データの結合
      cat_double(num_datas[cnt_chr] * NUMBER_OF_GABOR, ncv_data[cnt_chr], f_in_data);

      // 共通接続荷重による RFCN の局所受容野
      clEnqueueWriteBuffer(prop.queue, feature.cl_gtype, CL_TRUE,
          0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)pr_gtype[i],
          0, NULL, NULL);
      clEnqueueWriteBuffer(prop.queue, feature.cl_ncv_data, CL_TRUE,
          0, sizeof(double) * nsize * num_datas[cnt_chr] * NUMBER_OF_GABOR,
          (const void *)f_in_data, 0, NULL, NULL);
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
          feature_width, feature_height,
          f_out_data, f_data);


      // 加算出力データのプーリング
      clEnqueueWriteBuffer(prop.queue, pooling.cl_ncv, CL_TRUE,
          0, sizeof(double) * dsize * num_datas[cnt_chr],
          (const void *)f_data, 0, NULL, NULL);
      clEnqueueNDRangeKernel(prop.queue, prop.pooling, 3,
          NULL, (const size_t *)pooling_size, NULL,
          0, NULL, NULL);
      clEnqueueReadBuffer(prop.queue, pooling.cl_dest, CL_TRUE,
          0, sizeof(double) * psize * num_datas[cnt_chr],
          (void *)p_data, 0, NULL, NULL);

      // 三次元空間写像
      clEnqueueWriteBuffer(prop.queue, cls.cl_gtype,    CL_TRUE,
          0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)pr_class[i],
          0, NULL, NULL);
      clEnqueueWriteBuffer(prop.queue, cls.cl_ncv_data, CL_TRUE,
          0, sizeof(double) * psize * num_datas[cnt_chr],
          (const void *)p_data, 0, NULL, NULL);
      clEnqueueNDRangeKernel(prop.queue, prop.cls, 1,
          NULL, (const size_t *)&class_size, NULL,
          0, NULL, NULL);
      clEnqueueReadBuffer(prop.queue, cls.cl_vector, CL_TRUE,
          0, sizeof(cls_vector) * num_datas[cnt_chr],
          (void *)vector[cnt_chr], 0, NULL, NULL);
    } /* for(cnt_chr) */
    save_vector(i, num_datas, (const cls_vector **)vector);
  }



  cnt_generation = 0;
  do {
  } while(++cnt_generation != NUMBER_OF_GENERATION);

  printf("%s[Last Generation] %d%s\n", WHT_STR, cnt_generation, CLR_STR);


  
  /* 後処理 */
  bzero((void *)f_in_data,  sizeof(double) * num_datas[0] * NUMBER_OF_GABOR * nsize);
  bzero((void *)f_out_data, sizeof(double) * num_datas[0] * NUMBER_OF_GABOR * dsize);
  bzero((void *)f_data, sizeof(double) * num_datas[0] * dsize);
  bzero((void *)p_data, sizeof(double) * num_datas[0] * psize);
  free(f_out_data); f_out_data = NULL;
  free(f_in_data);  f_in_data  = NULL;
  free(f_data);     f_data     = NULL;
  free(p_data);     p_data     = NULL;

  genotype_destroy(POPULATION_SIZE, pr_gtype);
  genotype_destroy(POPULATION_SIZE, pr_class);

  cl_feature_destroy(&feature);
  cl_pooling_destroy(&pooling);
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

static cl_int
init_cl_pooling(cl_prop    *prop,
                cl_pooling *pooling,
                size_t     width,
                size_t     height,
                size_t     num_datas) {
  const size_t f_size = CanvasSize(width, height);
  const size_t p_size = CanvasSize((width / 2), (height / 2));
  cl_int status;

  pooling->cl_ncv  = clCreateBuffer(prop->context, CL_MEM_READ_ONLY,
      sizeof(double) * f_size * num_datas, (void *)NULL, &status);
  pooling->cl_dest = clCreateBuffer(prop->context, CL_MEM_WRITE_ONLY,
      sizeof(double) * p_size * num_datas, (void *)NULL, &status);

  status = clSetKernelArg(prop->pooling, 0, sizeof(size_t), (void *)&width);
  status = clSetKernelArg(prop->pooling, 1, sizeof(size_t), (void *)&height);
  status = clSetKernelArg(prop->pooling, 2, sizeof(cl_mem), (void *)&pooling->cl_ncv);
  status = clSetKernelArg(prop->pooling, 3, sizeof(cl_mem), (void *)&pooling->cl_dest);
  return status;
}

static cl_int
init_cl_class(cl_prop    *prop,
              cl_class   *cls,
              size_t     width,
              size_t     height,
              size_t     num_datas) {
  const size_t nsize = CanvasSize(width, height);

  cl_int status;

  cls->cl_gtype    = clCreateBuffer(prop->context, CL_MEM_READ_ONLY,
      sizeof(genotype_t) * MAX_GTYPE_SIZE, (void *)NULL, &status);
  cls->cl_ncv_data = clCreateBuffer(prop->context, CL_MEM_READ_ONLY,
      sizeof(double) * nsize * num_datas,  (void *)NULL, &status);
  cls->cl_vector   = clCreateBuffer(prop->context, CL_MEM_WRITE_ONLY,
      sizeof(double) * 3 * num_datas,      (void *)NULL, &status);

  status = clSetKernelArg(prop->cls, 0, sizeof(size_t), (void *)&width);
  status = clSetKernelArg(prop->cls, 1, sizeof(size_t), (void *)&height);
  status = clSetKernelArg(prop->cls, 2, sizeof(cl_mem), (void *)&cls->cl_gtype);
  status = clSetKernelArg(prop->cls, 3, sizeof(cl_mem), (void *)&cls->cl_ncv_data);
  status = clSetKernelArg(prop->cls, 4, sizeof(cl_mem), (void *)&cls->cl_vector);

  return status;
}

static void
cl_feature_destroy(cl_feature *feature) {
  clReleaseMemObject(feature->cl_dest);
  clReleaseMemObject(feature->cl_ncv_data);
  clReleaseMemObject(feature->cl_gtype);
}

static void
cl_pooling_destroy(cl_pooling *pooling) {
  clReleaseMemObject(pooling->cl_ncv);
  clReleaseMemObject(pooling->cl_dest);
}
