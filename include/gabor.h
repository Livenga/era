#ifndef __GABOR_H
#define __GABOR_H

#ifndef _MATH_H
#include <math.h>
#endif

#define NUMBER_OF_GABOR (4)

#define GABOR_X (20)
#define GABOR_Y GABOR_X

#define Deg2Rad(d) (((double)d / 180.0) * M_PI)


/* src/canvas/filter/f_gabor.c */
extern void
make_gabor(size_t width,
           size_t height,
           double theta,
           double psi,
           n_canvas *n_gabor);
#endif
