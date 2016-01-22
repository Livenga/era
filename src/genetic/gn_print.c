#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/genetic.h"

static void
draw_line(const size_t column);
static const char *
function_name(const int f_num);

void
gprint(size_t   num,
       genotype gtype) {
  extern int    spd[];
  extern double gain[], thr[], w[];

  int    i, j, position;
  size_t column, offset;

  column = HideSize(gtype[0]) + OutputSize(gtype[0]);

  printf("[Population Number]    %ld\n", num);
  printf("[Number of Transition] %llu\n", TransitionSize(gtype[0]));

  draw_line(column);
  printf("|           |");
  for(i = 0; i < HideSize(gtype[0]); i++)
    printf("    H%2d    |", i);
  for(i = 0; i < OutputSize(gtype[0]); i++)
    printf("    O%2d    |", i);
  printf(" 0x%010llX\n", gtype[0]);
  draw_line(column);

  printf("|  Function |");
  for(i = 0; i < column; i++) {
    position = (column - i - 1) * 4;
    printf(" %9s |", function_name((gtype[1] >> position) & 0x0F));
  }
  printf(" 0x%010llX\n", gtype[1]);

  printf("| Parameter |");
  for(i = 0; i < column; i++) {
    position = (column - i - 1) * 4;
    printf("   %+.2f   |", gain[(gtype[2] >> position) & 0x0F]);
  }
  printf(" 0x%010llX\n", gtype[2]);

  printf("|   Speed   |");
  for(i = 0; i < column; i++) {
    position = (column - i - 1) * 4;
    printf("    %3d    |", spd[(gtype[3] >> position) & 0x0F]);
  }
  printf(" 0x%010llX\n", gtype[3]);

  printf("| Threshold |");
  for(i = 0; i < column; i++) {
    position = (column - i - 1) * 4;
    printf("   %+.2f   |", thr[(gtype[4] >> position) & 0x0F]);
  }
  printf(" 0x%010llX\n", gtype[4]);
  draw_line(column);
  
  offset = ParameterSize(gtype[0]) + 1;
  for(i = 0; i < InternalSize(gtype[0]); i++) {
    printf("|    I%2d    |", i);

    for(j = 0; j < column; j++) {
      position = (column - j - 1) * 4;
      printf("   %+.2f   |", w[(gtype[offset + i] >> position) & 0x0F]);
    }
    printf(" 0x%010llX\n", gtype[offset + i]);
  }
  draw_line(column);

  offset += InternalSize(gtype[0]);

  if(ExternalSize(gtype[0]) != 0) {
    for(i = 0; i < ExternalSize(gtype[0]); i++) {
      printf("|    E%2d    |", i);

      for(j = 0; j < column; j++) {
        position = (column - j - 1) * 4;
        printf("   %+.2f   |", w[(gtype[offset + i] >> position) & 0x0F]);
      }
      printf(" 0x%010llX\n", gtype[offset + i]);
    }
    draw_line(column);
  }

  offset += ExternalSize(gtype[0]);
  for(i = 0; i < HideSize(gtype[0]); i++) {
    printf("|    H%2d    |", i);

    for(j = 0; j < column; j++) {
      position = (column - j - 1) * 4;
      printf("   %+.2f   |", w[(gtype[offset + i] >> position) & 0x0F]);
    }
    printf(" 0x%010llX\n", gtype[offset + i]);
  }
  draw_line(column);

  putchar('\n');
}

static void
draw_line(const size_t column) {
  int i;
  printf("+-----------+");
  for(i = 0; i < column; i++)
    printf("-----------+");
  putchar('\n');
}

static const char *
function_name(const int f_num) {
  const char *msg;

  switch(f_num) {
    case SIGMOID: msg = "Sigmoid"; break;
    default:      msg = "(nil)"; break;
  }

  return msg;
}
