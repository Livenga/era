#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "../../include/canvas.h"

/* 単色(GrayScale) PNG 画像読み込み */
int
pnread(const char *png_path,
       canvas *png_cv) {
  int i;

  FILE        *png_fp;
  png_structp png_ptr;
  png_infop   info_ptr;
  uchar       **png_data;


  if((png_fp = fopen(png_path, "rb")) == NULL) {
    perror(png_path);
    return EOF;
  }

  png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  
  png_init_io(png_ptr, png_fp);
  png_read_info(png_ptr, info_ptr);


  png_cv->width  = png_get_image_width(png_ptr, info_ptr);
  png_cv->height = png_get_image_height(png_ptr, info_ptr);
  png_cv->d      =
    (uchar *)calloc(CanvasSize(png_cv->width, png_cv->height), sizeof(uchar));

  png_data = (uchar **)calloc(png_cv->height, sizeof(uchar *));
  for(i = 0; i < png_cv->height; i++)
    png_data[i] = (uchar *)calloc(png_cv->width, sizeof(uchar));

  png_read_image(png_ptr, (png_bytepp)png_data);
  png_read_end(png_ptr, info_ptr);


  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(png_fp);

  for(i = 0; i < png_cv->height; i++) {
    memmove((void *)png_cv->d + (i * png_cv->width), (const void *)png_data[i],
        sizeof(uchar) * png_cv->width);
    bzero((void *)png_data[i], sizeof(uchar) * png_cv->width);
    free(png_data[i]); png_data[i] = NULL;
  }
  free(png_data); png_data = NULL;


  return 0;
}


/* 画像の書き込み */
int
pnwrite(const char *png_path,
        const canvas png_cv) {
  int i;

  FILE        *png_fp;
  png_structp png_ptr;
  png_infop   info_ptr;
  uchar       **png_data;


  if((png_fp = fopen(png_path, "wb")) == NULL) {
    perror(png_path);
    return EOF;
  }

  png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, png_fp);
  png_set_IHDR(png_ptr, info_ptr,
      png_cv.width, png_cv.height, 8, PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


  png_data = (uchar **)calloc(png_cv.height, sizeof(uchar *));
  for(i = 0; i < png_cv.height; i++) {
    png_data[i] = (uchar *)calloc(png_cv.width, sizeof(uchar));
    memmove((void *)png_data[i], (const void *)png_cv.d + (i * png_cv.width),
        sizeof(uchar) * png_cv.width);
  }


  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_bytepp)png_data);
  png_write_end(png_ptr, info_ptr);

  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);


  for(i = 0; i < png_cv.height; i++) {
    bzero((void *)png_data[i], sizeof(uchar) * png_cv.width);
    free(png_data[i]);
    png_data[i] = NULL;
  }

  free(png_data);
  png_data = NULL;

  fclose(png_fp);
  return 0;
}


/* 正規化画像の書き込み */
int
pnwrite_from_ncv(const char *png_path,
                 const n_canvas png_cv) {
  int    i, j;
  double val;

  FILE        *png_fp;
  png_structp png_ptr;
  png_infop   info_ptr;
  uchar       **png_data;


  if((png_fp = fopen(png_path, "wb")) == NULL) {
    perror(png_path);
    return EOF;
  }

  png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, png_fp);
  png_set_IHDR(png_ptr, info_ptr,
      png_cv.width, png_cv.height, 8, PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


  png_data = (uchar **)calloc(png_cv.height, sizeof(uchar *));
  for(i = 0; i < png_cv.height; i++) {
    png_data[i] = (uchar *)calloc(png_cv.width, sizeof(uchar));
    for(j = 0; j < png_cv.width; j++) {
      val = png_cv.n[i * png_cv.width + j];
      png_data[i][j] = (val > 1.0) ? 0xFF :
        (val < 0.0) ? 0.0 : (uchar)(val * 255.0);
    }
  }


  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_bytepp)png_data);
  png_write_end(png_ptr, info_ptr);

  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);


  for(i = 0; i < png_cv.height; i++) {
    bzero((void *)png_data[i], sizeof(uchar) * png_cv.width);
    free(png_data[i]);
    png_data[i] = NULL;
  }

  free(png_data);
  png_data = NULL;

  fclose(png_fp);
  return 0;
}


/* Gaborの書き込み */
int
pnwrite_from_gabor(const char *png_path,
                 const n_canvas png_cv) {
  int    i, j;
  double val;

  FILE        *png_fp;
  png_structp png_ptr;
  png_infop   info_ptr;
  uchar       **png_data;


  if((png_fp = fopen(png_path, "wb")) == NULL) {
    perror(png_path);
    return EOF;
  }

  png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, png_fp);
  png_set_IHDR(png_ptr, info_ptr,
      png_cv.width, png_cv.height, 8, PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


  png_data = (uchar **)calloc(png_cv.height, sizeof(uchar *));
  for(i = 0; i < png_cv.height; i++) {
    png_data[i] = (uchar *)calloc(png_cv.width, sizeof(uchar));
    for(j = 0; j < png_cv.width; j++) {
      val = png_cv.n[i * png_cv.width + j] + 0.5;
      png_data[i][j] = (val > 1.0) ? 0xFF :
        (val < 0.0) ? 0.0 : (uchar)(val * 255.0);
    }
  }


  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_bytepp)png_data);
  png_write_end(png_ptr, info_ptr);

  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);


  for(i = 0; i < png_cv.height; i++) {
    bzero((void *)png_data[i], sizeof(uchar) * png_cv.width);
    free(png_data[i]);
    png_data[i] = NULL;
  }

  free(png_data);
  png_data = NULL;

  fclose(png_fp);
  return 0;
}
