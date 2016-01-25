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
  const double threshold = 0.10000;
  extern int f_quit;

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
  double     center_euclid, ccenter_euclid;
  cls_vector *vector[A2Z], center_v[A2Z], ccenter_v;
  genotype   pr_gtype[POPULATION_SIZE], pr_class[POPULATION_SIZE]; // 個体集合
  genotype   ch_gtype[CHILDREN_SIZE],   ch_class[CHILDREN_SIZE];   // 子個体集合
  double     pr_fitness[POPULATION_SIZE], ch_fitness[CHILDREN_SIZE]; // 適応度

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

  /* 初期化 */
  memset((void *)pr_fitness, '\0', sizeof(pr_fitness));
  memset((void *)ch_fitness, '\0', sizeof(ch_fitness));


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

    /* 各文字の中央ベクトルの計算 */
    center_vector(num_datas, center_v, (const cls_vector **)vector);

    memset((void *)&ccenter_v, '\0', sizeof(ccenter_v));
    for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
      ccenter_v.x += center_v[cnt_chr].x;
      ccenter_v.y += center_v[cnt_chr].y;
      ccenter_v.z += center_v[cnt_chr].z;

      center_euclid = euclid_cchar2center(num_datas[cnt_chr],
          vector[cnt_chr], center_v[cnt_chr]);

      pr_fitness[i] += (center_euclid < threshold) ? 5.0 : center_euclid;
    }
    ccenter_v.x /= A2Z;
    ccenter_v.y /= A2Z;
    ccenter_v.z /= A2Z;
    ccenter_euclid = euclid_cchar2center(A2Z,
        (const cls_vector *)center_v, (const cls_vector)ccenter_v);

    pr_fitness[i] /= exp(ccenter_euclid);
    //pr_fitness[i] /= ((double)A2Z - ccenter_euclid);
    //pr_fitness[i] /= sqrt((double)A2Z * num_datas[0]) - ccenter_euclid;
    //if(pr_fitness[i] < threshold) pr_fitness[i] = 10.0;
  } /* for(i) */
  
  //save_ngtype(POPULATION_SIZE, 0, pr_gtype);
  //save_ngtype(POPULATION_SIZE, 1, pr_class);

  /* 子個体集合の割当 */
  for(i = 0; i < CHILDREN_SIZE; i++) {
    ch_gtype[i] = galloc(INTERNAL_SIZE, 0, 1);
    ch_class[i] = galloc(INTERNAL_SIZE, 0, 3);
  }


  // Minimal Generation Gap
  int slt_rand[2];
  int slt_best, slt_roulette;
  // 確認用
  int num_best;

  cnt_generation = 0;
  do {
    /* ランダムで交叉する個体を決定 */
    slt_rand[0] = rand() % POPULATION_SIZE;
    slt_rand[1] = rand() % POPULATION_SIZE;
    while(slt_rand[0] == slt_rand[1])
      slt_rand[1] = rand() % POPULATION_SIZE;


    /* 交叉 */
    gcrossover(CHILDREN_SIZE,
        pr_gtype[slt_rand[0]], pr_gtype[slt_rand[1]],
        ch_gtype);
    gcrossover(CHILDREN_SIZE,
        pr_class[slt_rand[0]], pr_class[slt_rand[1]],
        ch_class);

    
    memset((void *)ch_fitness, '\0', sizeof(ch_fitness));
    for(i = 0; i < CHILDREN_SIZE; i++) {
      for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
        // データの結合
        cat_double(num_datas[cnt_chr] * NUMBER_OF_GABOR, ncv_data[cnt_chr], f_in_data);
        // 共通接続荷重による RFCN の局所受容野
        clEnqueueWriteBuffer(prop.queue, feature.cl_gtype, CL_TRUE,
            0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)ch_gtype[i],
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

        // 三次元空間写像 (分類部)
        clEnqueueWriteBuffer(prop.queue, cls.cl_gtype,    CL_TRUE,
            0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)ch_class[i],
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

      /* 各文字の中央ベクトルの計算 */
      center_vector(num_datas, center_v, (const cls_vector **)vector);

      memset((void *)&ccenter_v, '\0', sizeof(ccenter_v));
      for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
        ccenter_v.x += center_v[cnt_chr].x;
        ccenter_v.y += center_v[cnt_chr].y;
        ccenter_v.z += center_v[cnt_chr].z;

        center_euclid = euclid_cchar2center(num_datas[cnt_chr],
            vector[cnt_chr], center_v[cnt_chr]);

        ch_fitness[i] += (center_euclid < threshold) ? 5.0 : center_euclid;
      }
      ccenter_v.x /= A2Z;
      ccenter_v.y /= A2Z;
      ccenter_v.z /= A2Z;

      ccenter_euclid = euclid_cchar2center(A2Z,
          (const cls_vector *)center_v, (const cls_vector)ccenter_v);

      ch_fitness[i] /= exp(ccenter_euclid);
      //ch_fitness[i] /= ((double)A2Z - ccenter_euclid);
      //ch_fitness[i] /= sqrt((double)A2Z) - ccenter_euclid;
      //ch_fitness[i] /= (sqrt((double)A2Z) * num_datas[0]) - ccenter_euclid;
      //if(ch_fitness[i] < threshold) ch_fitness[i] = 10.0;
    } /* for(i) */

    slt_best     = slt_best_ft(CHILDREN_SIZE, ch_fitness);
    slt_roulette = rand() % CHILDREN_SIZE;

    if((cnt_generation % 10) == 0) {
      //printf("%f, %f\n", ccenter_euclid, exp(ccenter_euclid));
      //printf("%f, %f\n", ccenter_euclid, (double)A2Z - ccenter_euclid);
      //printf("%f, %f\n", ccenter_euclid, sqrt((double)A2Z) - ccenter_euclid);
      printf("%f, %f\n", ccenter_euclid, sqrt((double)A2Z * num_datas[0]) - ccenter_euclid);
      printf("[経過世代数] %5d\n", cnt_generation);
      printf("[選択された位置と適応度]\n");
      printf("  %3d, %f\t%3d, %f\n\n",
          slt_rand[0], pr_fitness[slt_rand[0]],
          slt_rand[1], pr_fitness[slt_rand[1]]);
      printf("[子個体適応度, 最良値とランダム値]\n");
      printf("  %3d, %f\t%3d, %f\n\n",
          slt_best,     ch_fitness[slt_best],
          slt_roulette, ch_fitness[slt_roulette]);
      num_best = slt_best_ft(POPULATION_SIZE, pr_fitness);
      printf("[親個体集合, 最良適応度] %f\n", pr_fitness[num_best]);
    }
    if(f_quit) break; /* Ctrl-C 処理 */


    /* 適応度と遺伝子型の複製 */
    pr_fitness[slt_rand[0]] = ch_fitness[slt_best];
    pr_fitness[slt_rand[1]] = ch_fitness[slt_roulette];
    // 特徴抽出
    gcopy(pr_gtype[slt_rand[0]], ch_gtype[slt_best]);
    gcopy(pr_gtype[slt_rand[1]], ch_gtype[slt_roulette]);
    // 分類器
    gcopy(pr_class[slt_rand[0]], ch_class[slt_best]);
    gcopy(pr_class[slt_rand[1]], ch_class[slt_roulette]);

  } while(++cnt_generation != NUMBER_OF_GENERATION);


  printf("%s[最終世代] %d%s\n", WHT_STR, cnt_generation, CLR_STR);
  save_ngtype(POPULATION_SIZE, 0, (const genotype *)pr_gtype);
  save_ngtype(POPULATION_SIZE, 1, (const genotype *)pr_class);

  /* 適応度の最良値を選択 */
  num_best = slt_best_ft(POPULATION_SIZE, pr_fitness);

  for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
    // データの結合
    cat_double(num_datas[cnt_chr] * NUMBER_OF_GABOR, ncv_data[cnt_chr], f_in_data);
    // 共通接続荷重による RFCN の局所受容野
    clEnqueueWriteBuffer(prop.queue, feature.cl_gtype, CL_TRUE,
        0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)pr_gtype[num_best],
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

    // 三次元空間写像 (分類部)
    clEnqueueWriteBuffer(prop.queue, cls.cl_gtype,    CL_TRUE,
        0, sizeof(genotype_t) * MAX_GTYPE_SIZE, (const void *)pr_class[num_best],
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
  save_vector(num_best, num_datas, (const cls_vector **)vector);

  
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
