#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/canvas.h"

static int
roundup(double val) {
  int    int_val = (int)val;
  double point = val - (double)int_val;

  return int_val + ((point >= 0.5) ? 1 : 0);
}

void
cv2neighbor(size_t width,
            size_t height,
            canvas *cv) {
  const size_t src_size  = CanvasSize(cv->width, cv->height);
  const size_t dest_size = CanvasSize(width, height);

  int    x, y, xp, yp;
  double dx, dy;
  uchar  *src;


  /* 元データの保持 */
  src = (uchar *)calloc(src_size, sizeof(uchar));
  memmove((void *)src, (const void *)cv->d, src_size * sizeof(uchar));

  /* Canvas データの初期化 */
  bzero((void *)cv->d, src_size * sizeof(uchar));
  free(cv->d);
  cv->d = NULL;
  cv->d = (uchar *)calloc(dest_size, sizeof(uchar));

  dx = (double)width  / (double)cv->width;
  dy = (double)height / (double)cv->height;

  for(y = 0; y < height; y++) {
    yp = roundup((double)y / dy);
    for(x = 0; x < width; x++) {
      xp = roundup((double)x / dx);

      if(yp < cv->height && xp < cv->width) {
        cv->d[y * width + x] =
          src[yp * cv->width + xp];
      }

    }
  }

  cv->width  = width;
  cv->height = height;

  bzero((void *)src, src_size * sizeof(uchar));
  free(src);
  src = NULL;
}
