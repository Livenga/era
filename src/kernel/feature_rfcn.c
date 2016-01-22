#include <kernel_canvas.h>
#include <kernel_genetic.h>

__kernel void
feature_rfcn(__private size_t width,
             __private size_t height,
             __global  genotype_t *gtype,
             __global  double     *ncv_data,
             __global  double     *dest) {
  const int idx_x = get_global_id(0),
  idx_y = get_global_id(1),
  idx_z = get_global_id(2);

  /* 入出力データのオフセット */
  const size_t in_offset  = CanvasSize(width, height) * idx_z;
  const size_t out_offset = CanvasSize((width - 4), (height - 4)) * idx_z;
  /* 遺伝子型の横幅 */
  const size_t column     = HideSize(gtype[0]) + OutputSize(gtype[0]);

  const int index[5][5] = {
    {4, 5, 3, 5, 4},
    {5, 2, 1, 2, 5},
    {3, 1, 0, 1, 3},
    {5, 2, 1, 2, 5},
    {4, 5, 3, 5, 4}};

  /* 各々パラメータ */
  double gain[] = {0.25, 0.5, 1.0, 2.0},
         thr[]  = {-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0},
         w[]    = {-4.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 4.0};

  int    r_func[GSIZE], r_spd[GSIZE];
  double r_gain[GSIZE], r_thr[GSIZE], r_internal[GSIZE], r_external[GSIZE];
  double input_x, kernel_val;
  double tmp_output[GSIZE] = {0.0}, unit_output[GSIZE] = {0.0};

  int i, offset, position;
  int x, y, dx, dy;
  int cnt_spd, cnt_unit;

  /* パラメータの復元 */
  for(cnt_unit = 0; cnt_unit < column; cnt_unit++) {
    position = (column - cnt_unit - 1) * 4;
    r_func[cnt_unit] =      (gtype[1] >> position) & 0x0F;
    r_gain[cnt_unit] = gain[(gtype[2] >> position) & 0x0F];
    r_spd[cnt_unit]  =      (gtype[3] >> position) & 0x0F;
    r_thr[cnt_unit]  =  thr[(gtype[4] >> position) & 0x0F];
  }

  for(cnt_spd = 0; cnt_spd < 2; cnt_spd++) {
    for(cnt_unit = 0; cnt_unit < column; cnt_unit++) {
      position = (column - cnt_unit - 1) * 4;
      input_x  = 0.0;

      if(r_spd[cnt_unit] <= cnt_spd) {
        /* 近傍入力接続荷重の復元 */
        for(i = 0; i < InternalSize(gtype[0]); i++)
          r_internal[i] = w[(gtype[1 + ParameterSize(gtype[0]) + i] >> position) & 0x0F];

        for(y = 0; y < 5; y++) {
          dy = y + idx_y;
          for(x = 0; x < 5; x++) {
            dx = x + idx_x;
            input_x += r_internal[index[y][x]] *
              ncv_data[(dy * width + dx) + in_offset];
          }
        }

        offset = 1 + ParameterSize(gtype[0]) + InternalSize(gtype[0]) + ExternalSize(gtype[0]);
        for(i = 0; i < HideSize(gtype[0]); i++) {
          input_x += unit_output[cnt_unit] *
            w[(gtype[offset + i] >> position) & 0x0F];
        }

        input_x -= r_thr[cnt_unit];

        switch(r_func[cnt_unit]) {
          case SIGMOID: kernel_val = calc_sigmoid(r_gain[cnt_unit], input_x); break;
          default:      kernel_val = calc_sigmoid(r_gain[cnt_unit], input_x); break;
        }

        tmp_output[cnt_unit] = kernel_val;
      } /* if(cnt_spd) */
    } /* for(cnt_unit) */

    for(cnt_unit = 0; cnt_unit < column; cnt_unit++) {
      unit_output[cnt_unit] = tmp_output[cnt_unit];
    }
  } /* for(cnt_spd) */

  //printf("%f\n", unit_output[column - 1]);
  dest[out_offset + (idx_y * (width - 4) + idx_x)] = unit_output[column - 1];

}
