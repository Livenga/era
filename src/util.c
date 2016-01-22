#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "../include/canvas.h"

size_t *
numof_training_data(const char *d_name) {
  int    cnt_chr;
  char   train_path[1024], *p;
  size_t *num_trains;

  DIR           *train_dir;
  struct dirent *train_stts;

  num_trains = (size_t *)calloc(A2Z, sizeof(size_t));

  for(cnt_chr = 0; cnt_chr < A2Z; cnt_chr++) {
    bzero((void *)train_path, sizeof(train_path));
    sprintf(train_path, "%s/%c/", d_name, 'A' + cnt_chr);

    if((train_dir = opendir(train_path)) == NULL) {
      perror(train_path);
      free(num_trains);
      num_trains = NULL;
      return NULL;
    }

    while((train_stts = readdir(train_dir)) != NULL) {
      p = strstr(train_stts->d_name, ".png");
      if(p != NULL) ++num_trains[cnt_chr];
    }

    num_trains[cnt_chr] /= DIV;
    closedir(train_dir);
  }

  return num_trains;
}

double *
cat_data(size_t num_datas,
         n_canvas *data) {
  const size_t cv_size =
    CanvasSize(data[0].width, data[0].height);

  int    i;
  double *cat;

  cat = (double *)calloc(num_datas * cv_size, sizeof(double));
  for(i = 0; i < num_datas; i++)
    memmove((void *)(cat + (i * cv_size)),
        (const void *)data[i].n,
        cv_size * sizeof(double));

  return cat;
}
