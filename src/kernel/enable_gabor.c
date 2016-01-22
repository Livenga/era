#include <kernel_canvas.h>

__kernel void
enable_gabor(size_t gabor_width,
             size_t gabor_height,
             __global double *gabor,
             size_t ncv_width,
             size_t ncv_height,
             __global double *ncv,
             __global double *output
             ) {
  const int idx_x = get_global_id(0);
  const int idx_y = get_global_id(1);
  const int idx_z = get_global_id(2);

  const size_t gabor_offset =
    CanvasSize(gabor_width, gabor_height) * idx_z;
  const size_t ncv_offset   =
    CanvasSize(ncv_width, ncv_height)     * idx_z;

  int    x, y, dx, dy;
  int    x_offset, y_offset;
  double val;

  x_offset = (gabor_width  - 0) / 2;
  y_offset = (gabor_height - 0) / 2;

  val = 0.0;
  for(y = -y_offset; y < y_offset; y++) {
    dy = idx_y + y;

    for(x = -x_offset; x < x_offset; x++) {
      dx = idx_x + x;

      if(dy > -1 && dy < ncv_height &&
          dx > -1 && dx < ncv_width) {
        val +=
          gabor[((y + y_offset) * gabor_width + (x + x_offset)) + gabor_offset] *
          ncv[dy * ncv_width + dx];
#if 0
        printf("%+f, %+f, %+f\n",
            gabor[((y + y_offset) * gabor_width + (x + x_offset)) + gabor_offset],
            ncv[dy * ncv_width + dx],
            val);
#endif
      }

    }
  }

#if 0
  printf("%3dx%3dx%3d\t%3lux%3lu\t%3lux%3lu\n",
      idx_x, idx_y, idx_z, gabor_width, gabor_height, ncv_width, ncv_height);
#endif

  output[ncv_offset + (idx_y * ncv_width + idx_x)] = val;

}
