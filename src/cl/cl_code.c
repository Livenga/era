#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "../../include/opencl.h"


static size_t
numof_codes(const char *path) {
  size_t codes;
  char   *p;
  DIR    *kernel_dir;
  struct dirent *kernel_stts;

  if((kernel_dir = opendir(path)) == NULL) {
    perror(path);
    return 0;
  }

  codes = 0;
  while((kernel_stts = readdir(kernel_dir)) != NULL) {
    p = strstr(kernel_stts->d_name, ".c");
    if(p != NULL) ++codes;
  }

  closedir(kernel_dir);
  return codes;
}


int
get_kcode_path(const char  *path,
               kernel_code *kcode) {
  char   *p;
  size_t count;
  long   path_len;
  DIR    *kernel_dir;
  struct dirent *kernel_stts;

  if((kernel_dir = opendir(path)) == NULL) {
    perror(path);
    return EOF;
  }

  kcode->count = numof_codes(path);
  kcode->paths = (char **)calloc(kcode->count, sizeof(char *));

  count = 0;
  while((kernel_stts = readdir(kernel_dir)) != NULL) {
    p = strstr(kernel_stts->d_name, ".c");
    if(p != NULL) {
      path_len = strlen(path) + 1 + strlen(kernel_stts->d_name);
      kcode->paths[count] = (char *)calloc(path_len, sizeof(char));

      sprintf(kcode->paths[count], "%s/%s",
          path, kernel_stts->d_name);
      ++count;
    }
  }

  closedir(kernel_dir);
  return 0;
}

int
read_kcode(kernel_code *kcode) {
  FILE *kernel_fp;
  int  i;
  long kernel_len;

  kcode->codes = (char **)calloc(kcode->count, sizeof(char *));
  for(i = 0; i < kcode->count; i++) {
    if((kernel_fp = fopen(kcode->paths[i], "r")) == NULL) {
      perror(kcode->paths[i]);
      return EOF;
    }

    fseek(kernel_fp, 0L, SEEK_END);
    kernel_len = ftell(kernel_fp);
    fseek(kernel_fp, 0L, SEEK_SET);

    kcode->codes[i] = (char *)calloc(kernel_len, sizeof(char));
    fread((void *)kcode->codes[i], sizeof(char), kernel_len, kernel_fp);

    fclose(kernel_fp);
  }

  return 0;
}
