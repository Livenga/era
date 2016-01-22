#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "../../include/canvas.h"

void
canvas_destroy(canvas *cv) {
  bzero((void *)cv->d, sizeof(uchar) * CanvasSize(cv->width, cv->height));
  free(cv->d);
  cv->d = NULL;
}

void
n_canvas_destroy(n_canvas *ncv) {
  bzero((void *)ncv->n, sizeof(double) * CanvasSize(ncv->width, ncv->height));
  free(ncv->n);
  ncv->n = NULL;
}

n_canvas
ncvalloc(size_t width,
         size_t height) {
  n_canvas ncv;

  ncv.width  = width;
  ncv.height = height;
  ncv.n      = (double *)calloc(CanvasSize(width, height), sizeof(double));

  return ncv;
}
