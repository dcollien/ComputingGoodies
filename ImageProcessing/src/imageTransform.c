#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <wand/magick_wand.h>

#include "imageTransform.h"

unsigned char *get_imageTransform (
   pixelOperator transformer,
   unsigned char *imageBytes,
   size_t imageLength,
   size_t *newImageLength,
   void *imageParams
) {
   
   unsigned char *buffer;

   MagickWand *mw = NULL;
   PixelIterator *iterator = NULL;
   PixelWand **pixels = NULL;
   
   int x, y;
   double red, green, blue;
   size_t rowLength;

   size_t width, height;

   mw = NewMagickWand ();

   assert (MagickReadImageBlob (mw, (void *)imageBytes, imageLength));

   width = MagickGetImageWidth (mw);
   height = MagickGetImageHeight (mw);

   iterator = NewPixelIterator (mw);

   for (y = 0; y != height; ++y) {
      pixels = PixelGetNextIteratorRow (iterator, &rowLength);
      for (x = 0; x != width; ++x) {
         red = PixelGetRed (pixels[x]);
         green = PixelGetGreen (pixels[x]);
         blue = PixelGetBlue (pixels[x]);

         transformer (x, y, &red, &green, &blue, imageParams);

         PixelSetRed (pixels[x], red);
         PixelSetGreen (pixels[x], green);
         PixelSetBlue (pixels[x], blue);
      }
      PixelSyncIterator (iterator);
   }

   buffer = MagickGetImageBlob (mw, newImageLength);
   
   DestroyPixelIterator (iterator);
   DestroyMagickWand (mw);

   return buffer;
}

