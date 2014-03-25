#include <wand/magick_wand.h>

#include "imageGenerate.h"

unsigned char *get_imageGeneration (
   // input params
   const char *format,
   int width, int height,
   pixelOperator pixelGen,
   void *imageParams,
   
   // output param
   size_t *bufferLength
   ) {
   
   unsigned char *buffer;

   MagickWand *mw = NULL;
   PixelWand  *pw = NULL;
   PixelIterator *iterator = NULL;
   PixelWand **pixels = NULL;
   
   int x, y;
   double red, green, blue;
   size_t rowLength;

   mw = NewMagickWand ();
   
   pw = NewPixelWand ();
   PixelSetColor (pw, "white");
   MagickNewImage(mw, (size_t)width, (size_t)height, pw);
   DestroyPixelWand (pw);
   
   MagickSetImageFormat (mw, format);

   iterator = NewPixelIterator (mw);
   
   for (y = 0; y != height; ++y) {
      pixels = PixelGetNextIteratorRow (iterator, &rowLength);
      for (x = 0; x != width; ++x) {
         pixelGen (x, y, &red, &green, &blue, imageParams);

         PixelSetRed (pixels[x], red);
         PixelSetGreen (pixels[x], green);
         PixelSetBlue (pixels[x], blue);
      }
      PixelSyncIterator (iterator);
   }
   
   buffer = MagickGetImageBlob (mw, bufferLength);
   
   DestroyPixelIterator (iterator);
   DestroyMagickWand (mw);

   return buffer;
}
