#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/genetic.h"

genotype
galloc(size_t internal,
       size_t external,
       size_t output) {
  genotype gtype;

  gtype = (genotype)calloc(MAX_GTYPE_SIZE, sizeof(genotype_t));
  gtype[0]  = 0x0;
  gtype[0] += (1                 << (4 * 5));
  gtype[0] += (PARAMETER_SIZE    << (4 * 4));
  gtype[0] += (internal          << (4 * 3));
  gtype[0] += (external          << (4 * 2));
  gtype[0] += (DEFAULT_HIDE_SIZE << (4 * 1));
  gtype[0] += (output            << (4 * 0));

  return gtype;
}

void
ginit(genotype gtype) {
  extern int gain_size, spd_size, thr_size, w_size;

  int    i, j, val;
  size_t row, column;
  size_t position;


  row    = ParameterSize(gtype[0]) + InternalSize(gtype[0])
    + ExternalSize(gtype[0]) + HideSize(gtype[0]);
  column = HideSize(gtype[0])      + OutputSize(gtype[0]);

  for(i = 0; i < row; i++) {
    switch(i) {
      case 0:  val = SIGMOID + 1; break;
      case 1:  val = gain_size;   break;
      case 2:  val = spd_size;    break;
      case 3:  val = thr_size;    break;
      default: val = w_size;      break;
    }

    gtype[i + 1] = 0x0;
    for(j = 0; j < column; j++) {
      position = (column - j - 1) * 4;
      gtype[i + 1] += ((genotype_t)(rand() % val) << position);
    }
  }

}
