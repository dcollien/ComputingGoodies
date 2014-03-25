typedef struct overlay {
   long double x;
   long double y;
   int zoom;

   unsigned char *bytes;
   size_t length;

   struct overlay *next;
} overlay_t;

void overlayHiddenImages(
   overlay_t *overlays,

   long double x,
   long double y,
   int zoom,
   int imageWidth,
   int imageHeight,

   unsigned char **imageBufferPtr,
   size_t *length
);

overlay_t *initOverlays(const char *overlayPath);
void destroyOverlays(overlay_t *firstOverlay);
