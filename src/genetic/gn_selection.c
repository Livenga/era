#include <stdio.h>

int
slt_best_ft(size_t population,
            const double *ft) {
  int i, index = 0;
  double minof = ft[0];

  for(i = 1; i < population; i++) {
    if(minof > ft[i]) {
      minof = ft[i];
      index = i;
    }
  }
  return index;
}
