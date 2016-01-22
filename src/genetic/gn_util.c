#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/genetic.h"

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
