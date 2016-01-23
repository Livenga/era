#ifndef __GENETIC_H
#define __GENETIC_H

typedef unsigned long long int genotype_t;
typedef genotype_t * genotype;

/* 遺伝的アルゴリズムパラメータ */
#define NUMBER_OF_GENERATION (20000)
#define POPULATION_SIZE      (100)
#define CHILDREN_SIZE        (30)

/* 遺伝子型情報パラメータ */
#define HEADER_SIZE       (1)
#define PARAMETER_SIZE    (4)
#define INTERNAL_SIZE     (6)
#define EXTERNAL_SIZE     (1)
#define DEFAULT_HIDE_SIZE (6)
#define MAX_HIDE_SIZE     (sizeof(genotype_t) * 8 - DEFAULT_HIDE_SIZE)
#define EXTENDED_SIZE     (MAX_HIDE_SIZE - DEFAULT_HIDE_SIZE)

#define DEFAULT_GTYPE_SIZE (HEADER_SIZE + PARAMETER_SIZE + \
    INTERNAL_SIZE + EXTERNAL_SIZE + DEFAULT_HIDE_SIZE)
#define MAX_GTYPE_SIZE     (DEFAULT_GTYPE_SIZE + EXTENDED_SIZE)

/* 各パラメータ数 */
#define TransitionSize(m) ((m >> 20) & 0x0F)
#define ParameterSize(m)  ((m >> 16) & 0x0F)
#define InternalSize(m)   ((m >> 12) & 0x0F)
#define ExternalSize(m)   ((m >>  8) & 0x0F)
#define HideSize(m)       ((m >>  4) & 0x0F)
#define OutputSize(m)     ((m >>  0) & 0x0F)

/* 関数番号 */
enum {
  SIGMOID = 0, THRESHOLD
} func_number;


#if defined(__MAIN_EVOLUTION__)
double gain[] = {0.25, 0.5, 1.0, 2.0},
       thr[]  = {-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0},
       w[]    = {-4.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 4.0};
int spd[]     = {0, 1};
int gain_size = sizeof(gain) / sizeof(gain[0]),
    thr_size  = sizeof(thr)  / sizeof(thr[0]),
    w_size    = sizeof(w)    / sizeof(w[0]),
    spd_size  = sizeof(spd)  / sizeof(spd[0]);
#endif


/* src/genetic/gn_init.c */
extern genotype
galloc(size_t internal,
       size_t external,
       size_t output);
extern void
ginit(genotype gtype);


/* src/genetic/gn_util.c */
extern void
genotype_destroy(size_t   population,
                 genotype *gtype);


/* src/genetic/gn_print.c */
extern void
gprint(size_t   num,
       genotype gtype);



typedef struct _cls_vector {
  double x, y, z;
} cls_vector;

/* src/vector.c */
extern void
save_vector(size_t num,
            const size_t     *num_datas,
            const cls_vector **vector);

#endif
