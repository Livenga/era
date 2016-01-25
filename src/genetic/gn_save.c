#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../include/genetic.h"

static void
get_date(char *data_str);

int
save_ngtype(size_t population,
            const  int type, /* 0:feature, 1:class */
            const  genotype *gtype) {
  int  i;
  char dates[256];

  FILE   *gtype_fp;
  char   rfcn_sign[4], gtype_path[1024];
  size_t gtype_t_size, population_size;
  size_t gtype_row;


  bzero((void *)rfcn_sign,  sizeof(rfcn_sign));
  bzero((void *)dates,      sizeof(dates));
  bzero((void *)gtype_path, sizeof(gtype_path));

  strcpy(rfcn_sign, (const char *)RFCN_SIGN);

  /* YYYYMMDD_hhmmss 時間文字列 */
  get_date(dates);
  if(strlen(dates) < 2) return EOF;

  /* Genotype ファイルパスの取得 */
  sprintf(gtype_path, "./WorkSpace/gtype/gtype_%s_%s.gtype",
      (type == 0 ? "feature" : "class"), dates);


  if((gtype_fp = fopen(gtype_path, "w")) == NULL) {
    perror(gtype_path);
    return EOF;
  }

  /* 遺伝子型の情報 */
  gtype_t_size    = sizeof(genotype_t);
  population_size = population;

  fwrite((const void *)rfcn_sign,        sizeof(char),   4, gtype_fp);
  fwrite((const void *)&gtype_t_size,    sizeof(size_t), 1, gtype_fp);
  fwrite((const void *)&population_size, sizeof(size_t), 1, gtype_fp);

  for(i = 0; i < population; i++) {
    gtype_row = 1 + ParameterSize(gtype[i][0]) + InternalSize(gtype[i][0]) +
      ExternalSize(gtype[i][0]) + HideSize(gtype[i][0]);

    fwrite((const void *)gtype[i], sizeof(genotype_t), gtype_row, gtype_fp);
  }

  fclose(gtype_fp);


  return 0;
}


static void
get_date(char *data_str) {
  time_t d_now;
  struct tm *local_str;

  d_now = time(NULL);

  if((local_str = localtime((const time_t *)&d_now)) != NULL) {
    sprintf(data_str, "%04d%02d%02d_%02d%02d%02d",
        local_str->tm_year + 1900, local_str->tm_mon + 1,
        local_str->tm_mday,        local_str->tm_hour,
        local_str->tm_min,         local_str->tm_sec);
  } else {
    data_str = '\0';
  }
}
