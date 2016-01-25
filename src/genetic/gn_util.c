#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/genetic.h"

void
gcopy(genotype dest,
      const genotype src) {
  int i;
  size_t row = 1 + ParameterSize(src[0]) + InternalSize(src[0]) +
    ExternalSize(src[0]) + HideSize(src[0]);

  for(i = 0; i < row; i++) {
    dest[i] = 0x0;
    dest[i] = src[i];
  }
}


void
genotype_destroy(size_t population,
                 genotype *gtype) {
  int i;

  for(i = 0; i < population; i++) {
    bzero((void *)gtype[i], sizeof(genotype_t) * MAX_GTYPE_SIZE);
    free(gtype[i]);
    gtype[i] = NULL;
  }
}
