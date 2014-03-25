#include <wand/magick_wand.h>
#include <assert.h>

#include "imageOverlay.h"

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
   ) {
   
   unsigned char *buffer = NULL;

   char *format;

   int overlayWidth = 0;
   int overlayHeight = 0;

   int newWidth = 0;
   int newHeight = 0;
   
   MagickWand *imageWand = NULL;
   MagickWand *overlayWand = NULL;

   PixelWand  *pw = NULL;

   imageWand = NewMagickWand ();
   overlayWand = NewMagickWand ();

   // load images into wands
   assert (overlayBytes != NULL);
   assert (MagickReadImageBlob (overlayWand, (void *)overlayBytes, overlayLength));

   overlayWidth = MagickGetImageWidth (overlayWand);
   overlayHeight = MagickGetImageHeight (overlayWand);

   if (imageBytes != NULL) {
      assert (MagickReadImageBlob (imageWand, (void *)imageBytes, imageLength));
   } else {
      // imageBytes can be NULL (for just scaling an overlay on its own canvas, relative to its original self)

      pw = NewPixelWand ();
      PixelSetColor (pw, "white");
      MagickNewImage (imageWand, (size_t)overlayWidth, (size_t)overlayHeight, pw);
      DestroyPixelWand (pw);

      format = MagickGetImageFormat (overlayWand);
      printf ("Set format to: %s\n", format);
      MagickSetImageFormat (imageWand, format);
      free (format);
   }

   // scale overlay
   newWidth = (overlayWidth * scale);
   newHeight = (overlayHeight * scale);

   assert (MagickResizeImage (overlayWand, newWidth, newHeight, BoxFilter, 0));

   // composite images
   assert (MagickCompositeImage (imageWand, overlayWand, OverCompositeOp, offsetX, offsetY));

   buffer = MagickGetImageBlob (imageWand, bufferLength);
   assert (buffer != NULL);
   
   DestroyMagickWand (imageWand);
   DestroyMagickWand (overlayWand);

   return buffer;
}


void get_imageDimensions (unsigned char *bytes, size_t length, int *width, int *height) {
   MagickWand *imageWand = NULL;
   imageWand = NewMagickWand ();

   assert (MagickReadImageBlob (imageWand, (void *)bytes, length));

   *width  = MagickGetImageWidth (imageWand);
   *height = MagickGetImageHeight (imageWand);

   DestroyMagickWand (imageWand);
}

