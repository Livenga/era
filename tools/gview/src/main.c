#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/genetic.h"

int main(int argc, char *argv[])
{
  int i, j;

  FILE   *gtype_fp;
  char   rfcn_sign[4];
  size_t rfcn_pop_size, gtype_t_size;

  genotype_t ghead;
  genotype   gtype;
  size_t     gtype_row;


  if(argc < 2) {
    fprintf(stderr, "%s: \033[1;5mOperation Error.\033[0m\n", argv[0]);
    fprintf(stderr, "Usage '%s [RFCN File]'\n", argv[0]);
    return EOF;
  }

  bzero((void *)rfcn_sign, sizeof(rfcn_sign));
  if((gtype_fp = fopen(argv[1], "r")) == NULL) {
    perror(argv[1]);
    return EOF;
  }

  fread((void *)rfcn_sign,      sizeof(char),   4, gtype_fp);
  fread((void *)&gtype_t_size,  sizeof(size_t), 1, gtype_fp);
  fread((void *)&rfcn_pop_size, sizeof(size_t), 1, gtype_fp);

  printf("[RFCN Signature]      %s\n", rfcn_sign);
  printf("[RFCN Information]    genotype_t... %ld\n", gtype_t_size);
  printf("[RFCN Populaton Size] %ld\n", rfcn_pop_size);

  for(i = 0; i < rfcn_pop_size; i++) {
    fread((void *)&ghead, gtype_t_size, 1, gtype_fp);
    gtype_row = RowSize(ghead);

    gtype = (genotype)calloc(gtype_row, sizeof(genotype_t));
    fread((void *)gtype, gtype_t_size, gtype_row, gtype_fp);

    printf("0x%010llX\n", ghead);
    for(j = 0; j < gtype_row; j++)
      printf("0x%010llX\n", gtype[j]);
    putchar('\n');

    bzero((void *)gtype, gtype_t_size * gtype_row);
    free(gtype);
  }

  fclose(gtype_fp);
	return 0;
}
