#include <kernel_canvas.h>

__kernel void
enable_pooling(size_t          width,
               size_t          height,
               __global double *ncv,
               __global double *dest) {
  const int idx_x = get_global_id(0);
  const int idx_y = get_global_id(1);
  const int idx_z = get_global_id(2);

  const size_t  ncv_offset =
    CanvasSize(width, height) * idx_z;
  const size_t dest_offset =
    CanvasSize((width / 2), (height / 2)) * idx_z;

  int    x, y, dx, dy;
  double val;

  val = 0.0;
  for(y = 0; y < 2; y++) {
    dy = y + (idx_y * 2);
    for(x = 0; x < 2; x++) {
      dx = x + (idx_x * 2);
      val += ncv[ncv_offset + (dy * width + dx)];
    }
  }

  //printf("%lux%lu\t%dx%dx%d\n", width, height, idx_x, idx_y, idx_z);

  dest[dest_offset + (idx_y * (width / 2) + idx_x)] = val / 4.0;

}
