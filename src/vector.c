#include <stdio.h>
#include "../include/canvas.h"
#include "../include/genetic.h"

void
save_vector(size_t num,
            const size_t     *num_datas,
            const cls_vector **vector) {
  FILE *csv_fp;
  char csv_path[1024];

  int i, j;

  sprintf(csv_path, "./WorkSpace/csv/%04lu.csv", num);
  if((csv_fp = fopen(csv_path, "w")) != NULL) {

    for(i = 0; i < A2Z; i++) {
      for(j = 0; j < num_datas[i]; j++)
        fprintf(csv_fp, "%f, %f, %f\n",
            vector[i][j].x, vector[i][j].y, vector[i][j].z);
      fprintf(csv_fp, "\n");
    }

    fclose(csv_fp);
  }
}
