#ifndef __GENETIC_H
#define __GENETIC_H

typedef unsigned long long int genotype_t;
typedef genotype_t * genotype;

/* 各パラメータ数 */
#define TransitionSize(m) ((m >> 20) & 0x0F)
#define ParameterSize(m)  ((m >> 16) & 0x0F)
#define InternalSize(m)   ((m >> 12) & 0x0F)
#define ExternalSize(m)   ((m >>  8) & 0x0F)
#define HideSize(m)       ((m >>  4) & 0x0F)
#define OutputSize(m)     ((m >>  0) & 0x0F)

#define SIGMOID   (0)
#define THRESHOLD (1)

#define GSIZE     (32)

double
calc_sigmoid(double alpha,
             double input_x);

#endif
