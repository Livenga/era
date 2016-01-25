#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../include/canvas.h"
#include "../include/genetic.h"

//#define __VIEW_VECOTR__

void
center_vector(const size_t     *num_datas,
              cls_vector       *center_v,
              const cls_vector **vectors) {
  int i, j;

  for(i = 0; i < A2Z; i++) {
    center_v[i].x = 0.0;
    center_v[i].y = 0.0;
    center_v[i].z = 0.0;

    for(j = 0; j < num_datas[i]; j++) {
      center_v[i].x += vectors[i][j].x;
      center_v[i].y += vectors[i][j].y;
      center_v[i].z += vectors[i][j].z;
    }

    center_v[i].x /= num_datas[i];
    center_v[i].y /= num_datas[i];
    center_v[i].z /= num_datas[i];

#if defined(__VIEW_VECOTR__)
    printf("'%c' %f, %f, %f\n", 'A' + i,
        center_v[i].x, center_v[i].y, center_v[i].z);
#endif
  }
}

double
euclid_cchar2center(size_t num,
                    const cls_vector *char_pt,
                    const cls_vector center_pt) {
  int    i;
  double euclid = 0.0;

  for(i = 0; i < num; i++) {
    euclid += sqrt(
        pow((char_pt[i].x - center_pt.x), 2.0) +
        pow((char_pt[i].y - center_pt.y), 2.0) +
        pow((char_pt[i].z - center_pt.z), 2.0));
  }

  return euclid;
}


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
