#include "imageInit.h"
#include <wand/magick_wand.h>

void init_imageTools (void) {
   MagickWandGenesis ();
}

void deinit_imageTools (void) {
   MagickWandTerminus ();
}
