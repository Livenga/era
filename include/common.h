#ifndef _COMMON_H
#define _COMMON_H

#define ERR_STR "\033[1;5;31m"
#define GRN_STR "\033[1;32m"
#define WHT_STR "\033[1;37m"
#define CLR_STR "\033[0;39m"

#ifndef _CANVAS_H
#include "canvas.h"
#endif

/* src/util.c */
extern size_t *
numof_training_data(const char *d_name);
extern double *
cat_data(size_t num_datas,
         n_canvas *data);
#endif
