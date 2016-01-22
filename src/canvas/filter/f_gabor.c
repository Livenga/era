#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../../include/canvas.h"
#include "../../../include/gabor.h"

static double
calc_sigma(double b,
           double lambda);


void
make_gabor(size_t width,
           size_t height,
           double theta,
           double psi,
           n_canvas *dest) {
  int x, y;
  int x_offset, y_offset;
  int position;

  double dx, dy;
  double gamma, sigma, lambda, b;
  double exp_p, cos_p;

  /* Gabor パラメータの設定 */
#if 1
  b      = 0.4; /* Div */
  gamma  = 2.3; /* Times */
  lambda = 1.8; /* Div */
  /* b = 0.4, gamma = 2.3, lambda = 1.8 * - 2016/01/07 */
  /* b = 0.5, gamma = 1.8, lambda = 2.3 */
#else
  b      = 0.8;
  gamma  = 1.0;
  lambda = 4.0;
#endif
  sigma    = calc_sigma(b, lambda);

  x_offset = (width  - 0) / 2;
  y_offset = (height - 0) / 2;

  /* n_canvas の初期化 */
  dest->width  = width;
  dest->height = height;
  dest->n      =
    (double *)calloc(width * height, sizeof(double));

  for(y = -y_offset; y < y_offset; y++) {
    for(x = -x_offset; x < x_offset; x++) {
      position = (y + y_offset) * width + (x + x_offset);

      dx =  (double)x * cos(Deg2Rad(theta)) + (double)y * sin(Deg2Rad(theta));
      dy = -(double)x * sin(Deg2Rad(theta)) + (double)y * cos(Deg2Rad(theta));

      exp_p = (pow(dx, 2.0) + pow(gamma, 2.0) * pow(dy, 2.0))
        / (2.0 * pow(sigma, 2.0));
      cos_p = (2.0 * M_PI * (dx / lambda)) + Deg2Rad(psi);

      dest->n[position] = exp(-exp_p) * cos(cos_p);
    }
  }

}


static double
calc_sigma(double b,
           double lambda) {
  return ((1.0 / M_PI) * sqrt(log(2.0) / 2.0) *
    ((pow(2.0, b) + 1.0) / (pow(2.0, b) - 1.0))) * lambda;
}
