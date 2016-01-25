#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/genetic.h"

static genotype_t
get_gate(const size_t column);

void
gcrossover(size_t population,
           genotype src0,
           genotype src1,
           genotype *dest) {
  const size_t row = 1 + ParameterSize(src0[0]) + InternalSize(src0[0])
    + ExternalSize(src0[0]) + HideSize(src0[0]),
  column = HideSize(src0[0]) + OutputSize(src0[0]);

  int i;
  int cnt_gen;

  size_t x_pt, y_pt;

  genotype_t gate;

  for(cnt_gen = 0; cnt_gen < population; cnt_gen += 2) {
    x_pt = (rand() % (column - 1)) + 1;
    y_pt = (rand() % (row    - 1)) + 1;

    dest[cnt_gen + 0][0] = src0[0];
    dest[cnt_gen + 1][0] = src0[0];

    gate = get_gate(x_pt);

    for(i = 1; i < y_pt; i++) {
      dest[cnt_gen + 0][i] = (src0[i] & gate) + (src1[i] & ~gate);
      dest[cnt_gen + 1][i] = (src1[i] & gate) + (src0[i] & ~gate);
    }

    for(; i < row; i++) {
      dest[cnt_gen + 0][i] = (src1[i] & gate) + (src0[i] & ~gate);
      dest[cnt_gen + 1][i] = (src0[i] & gate) + (src1[i] & ~gate);
    }

  } /* for(cnt_gen) */

}

static genotype_t
get_gate(const size_t column) {
  int i;
  genotype_t gate = 0x0;
  for(i = 0; i < column; i++)
    gate += ((genotype_t)0x0F << (i * 4));
  return gate;
}
