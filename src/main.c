#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "../include/opencl.h"
#include "../include/common.h"
#include "../include/canvas.h"
#include "../include/util.h"

// Ctrl-C シグナルの変更
int f_quit;
void
quit_signal(int sig);


int
main(int argc,
     char *argv[]) {
  const int pid = 0, did = 0;
  cl_prop   prop;
  
  int      cnt_chr, cnt_data, cnt_gabor;
  canvas   *cv_chr;
  n_canvas *ncv_chr;
  n_canvas *l_gabor_ptr;
  n_canvas *l_gabor[A2Z], *l_pooling[A2Z];
  n_canvas gabor[NUMBER_OF_GABOR];
  char     cv_path[1024];

  size_t   *num_datas;

  /* OpenCL Device 設定 */
  get_kcode_path("./src/kernel", &prop.kcode);
  read_kcode(&prop.kcode);

  get_platforms(&prop);
  get_devices(pid, &prop);
  set_kernel(did, &prop);
  print_platform_info(prop);
  print_device_info(prop);


  /* ランダムシード, 強制終了シグナル */
  srand((unsigned)time(NULL));
  signal(SIGINT, quit_signal);


  /* 学習データ数の取得 */
  num_datas = numof_training_data("./training_datas");
  /* Gabor 特徴量層の領域確保 */
  for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
    l_gabor[cnt_chr]   =
      (n_canvas *)calloc(num_datas[cnt_chr] * NUMBER_OF_GABOR, sizeof(n_canvas));
    l_pooling[cnt_chr] =
      (n_canvas *)calloc(num_datas[cnt_chr] * NUMBER_OF_GABOR, sizeof(n_canvas));

    for(cnt_data = 0; cnt_data < num_datas[cnt_chr]; cnt_data++) {
      for(cnt_gabor = 0; cnt_gabor < NUMBER_OF_GABOR; cnt_gabor++) {
        l_gabor[cnt_chr][cnt_data * NUMBER_OF_GABOR + cnt_gabor] =
          ncvalloc(CANVAS_X, CANVAS_Y);
        l_pooling[cnt_chr][cnt_data * NUMBER_OF_GABOR + cnt_gabor] =
          ncvalloc(CANVAS_X / 2, CANVAS_Y / 2);
      }
    }
  }

  /* NUMBER_OF_GABOR 個の Gabor Filter */
  for(cnt_gabor = 0; cnt_gabor < NUMBER_OF_GABOR; cnt_gabor++) {
    make_gabor(GABOR_X, GABOR_Y, cnt_gabor * (180.0 / NUMBER_OF_GABOR),
        0.0, &gabor[cnt_gabor]);
  }


  /* 特徴量抽出 - 前処理 */
  for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
    cv_chr  =   (canvas *)calloc(num_datas[cnt_chr], sizeof(canvas));
    ncv_chr = (n_canvas *)calloc(num_datas[cnt_chr], sizeof(n_canvas));

    l_gabor_ptr = l_gabor[cnt_chr];

    /* 'A' + cnt_chr データの読み込み */
    for(cnt_data = 0; cnt_data < num_datas[cnt_chr]; cnt_data++) {
      sprintf(cv_path, "./training_datas/%c/%c_%04d.png",
          'A' + cnt_chr, 'A' + cnt_chr, cnt_data);
      pnread(cv_path, &cv_chr[cnt_data]);
      cv2reverse(&cv_chr[cnt_data]);
      cv2neighbor(CANVAS_X, CANVAS_Y, &cv_chr[cnt_data]);
      cv2normalize(&ncv_chr[cnt_data], cv_chr[cnt_data]);

      do_gabor(prop, ncv_chr[cnt_data], gabor, l_gabor_ptr);
      l_gabor_ptr += NUMBER_OF_GABOR;
    }

    do_pooling(prop, num_datas[cnt_chr], l_gabor[cnt_chr], l_pooling[cnt_chr]);

    for(cnt_data = 0; cnt_data < num_datas[cnt_chr]; cnt_data++) {
      canvas_destroy(&cv_chr[cnt_data]);
      n_canvas_destroy(&ncv_chr[cnt_data]);
    }

    printf("[Enable feature extraction alphabet] '%c'\t%d Datas\t[%sDone%s]\n",
        'A' + cnt_chr, cnt_data, GRN_STR, CLR_STR);
    fflush(stdout);
  } /* for(cnt_chr) */


  printf("\n\n\033[1;5;37m *** Start evolution calculate *** \033[0;39m\n\n");
  evolution(prop, num_datas, (const n_canvas **)l_pooling);


  /* 後処理 */
  int position;
  for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
    for(cnt_data = 0; cnt_data < num_datas[cnt_chr]; cnt_data++) {
      for(cnt_gabor = 0; cnt_gabor < NUMBER_OF_GABOR; cnt_gabor++) {
        position = cnt_data * NUMBER_OF_GABOR + cnt_gabor;
        n_canvas_destroy(&l_gabor[cnt_chr][position]);
        n_canvas_destroy(&l_pooling[cnt_chr][position]);
      }
    }
    free(l_pooling[cnt_chr]); l_pooling[cnt_chr] = NULL;
    free(l_gabor[cnt_chr]);   l_gabor[cnt_chr]   = NULL;
  }

  bzero((void *)num_datas, sizeof(size_t) * A2Z);
  free(num_datas); num_datas = NULL;

  for(cnt_gabor = 0; cnt_gabor < NUMBER_OF_GABOR; cnt_gabor++)
    n_canvas_destroy(&gabor[cnt_gabor]);
  cl_prop_destroy(&prop);

  return 0;
}

void
quit_signal(int sig) {
  f_quit = 1;
  fprintf(stderr, "%s*** Forced Termination ***%s\n", ERR_STR, CLR_STR);
  signal(SIGINT, SIG_DFL);
}
