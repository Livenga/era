#ifndef __CANVAS_H
#define __CANVAS_H

#define A2Z      ('Z' - 'A') + 1
#define DIV      (10)

#define CANVAS_X (28)
#define CANVAS_Y CANVAS_X

#define CanvasSize(x, y) (x * y)

typedef unsigned char uchar;

typedef struct _canvas {
  char   ch;
  size_t width, height;
  uchar  *d;
} canvas;

typedef struct _n_canvas {
  char   ch;
  size_t width, height;
  double *n;
} n_canvas;


#ifndef _GABOR_H
#include "gabor.h"
#endif


/* src/canvas/cv_png.c */
extern int
pnread(const char *png_path,
       canvas *png_cv);
extern int
pnwrite(const char *png_path,
        const canvas png_cv);
extern int
pnwrite_from_ncv(const char *png_path,
                 const n_canvas png_cv);
extern int
pnwrite_from_gabor(const char *png_path,
                 const n_canvas png_cv);

/* src/canvas/cv_conv.c */
extern void
cv2reverse(canvas *cv);
extern void
cv2normalize(n_canvas *dest,
             const canvas src);


/* src/canvas/cv_resize.c */
extern void
cv2neighbor(size_t width,
            size_t height,
            canvas *cv);


/* src/canvas/cv_util.c */
extern n_canvas
ncvalloc(size_t width,
         size_t height);
extern void
canvas_destroy(canvas *cv);
extern void
n_canvas_destroy(n_canvas *ncv);

#endif
