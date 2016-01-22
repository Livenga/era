#ifndef __UTIL_H
#define __UTIL_H
/* src/evolution.c */
extern void
evolution(cl_prop        prop,
          const size_t   *num_datas,
          const n_canvas **ncv_data);

/* src/gabor.c */
extern void
do_gabor(cl_prop prop,
         n_canvas n_data,
         n_canvas *n_gabor,
         n_canvas *dest);
/* src/pooling.c */
extern void
do_pooling(cl_prop  prop,
           size_t   num_datas,
           n_canvas *ncv,
           n_canvas *n_dest);
#endif
