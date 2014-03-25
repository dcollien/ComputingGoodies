#ifndef IMAGE_GENERATE_H
#define IMAGE_GENERATE_H

#include "imageInit.h"

// Generate an image from imageParams, 
// by feeding each pixel through a pixelGen function
unsigned char *get_imageGeneration (
   // input params
   const char *format,
   int width, int height,
   pixelOperator pixelGen,
   void *imageParams,
   
   // output param
   size_t *bufferLength
);

#endif
