#ifndef IMAGE_OVERLAY_H
#define IMAGE_OVERLAY_H

#include "imageInit.h"

unsigned char *get_overlayedImage (
   // input params
   unsigned char *imageBytes,
   size_t imageLength,

   unsigned char *overlayBytes,
   size_t overlayLength,

   int offsetX, int offsetY,
   double scale,

   // output param
   size_t *bufferLength
);

void get_imageDimensions (unsigned char *bytes, size_t length, int *width, int *height);

#endif
