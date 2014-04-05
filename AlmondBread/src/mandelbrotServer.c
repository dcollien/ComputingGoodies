#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include <sys/stat.h>

#include "MandelbrotSet.h"
#include "mandelbrotServer.h"
#include "puzzleQuest.h"

#define MAX_FILE_SIZE 5242880

#define CACHE_PATH "../imageCache"
#define CACHE_PATH_LEN 13

#define OVERLAY_PATH "../overlays"
#define OVERLAY_PATH_LEN 11

#define LINE_SIZE 1024

#define MAX_OVERLAYS 127

static color colorPalette[DEFAULT_MAX_ITERATIONS+1] = {};
static overlay_t *overlays;

void initMandelbrot(FILE *colorFile) {
   char line[LINE_SIZE];
   int r, g, b;
   struct stat dirStat;
   
   if (stat(CACHE_PATH, &dirStat) < 0 || !S_ISDIR(dirStat.st_mode)) {
      fprintf(stderr, "Cache directory: " CACHE_PATH " does not exist\n");
      exit(EXIT_FAILURE);
   }
   
   if (stat(OVERLAY_PATH, &dirStat) < 0 || !S_ISDIR(dirStat.st_mode)) {
      fprintf(stderr, "Overlay directory: " OVERLAY_PATH " does not exist\n");
      exit(EXIT_FAILURE);
   }
   
   overlays = initOverlays(OVERLAY_PATH);
   
   long double maxVal = 255.0;
   int i = 0;
   for (i = 0; i <= DEFAULT_MAX_ITERATIONS; i++) {
      if (fgets(line, sizeof line, colorFile) != NULL) {
         sscanf(line, "%d %d %d", &r, &g, &b);
         colorPalette[i].red = r / maxVal;
         colorPalette[i].green = g / maxVal;
         colorPalette[i].blue = b / maxVal;
      }
   }
}

void deinitMandelbrot(void) {
   destroyOverlays(overlays);
}

void saveToCache(char *requestURI, unsigned char *imageBuffer, int length) {
   FILE *cacheFile;
   char cachePath[MAX_URI_SIZE+CACHE_PATH_LEN] = {0};
   char uri[MAX_URI_SIZE] = {0};
   
   strncpy(uri, requestURI, MAX_URI_SIZE);
   sprintf(cachePath, "%s%s", CACHE_PATH, uri);

   cacheFile = fopen(cachePath, "wb");
   fwrite(imageBuffer, 1, length, cacheFile);
   fclose(cacheFile);
}

unsigned char *serveMandelbrot(char *requestURI, int width, int height, char *extension, size_t *length) {
   unsigned char *imageBuffer = NULL;
   MandelbrotSet fractal;

   char *imageFormat = NULL;

   int scannedFields;

   bool isValidURI = false;

   int zoom = 8;
   mandelbrotCoord center = {-0.5, -0.0};

   FILE *cacheFile;
   char cachePath[MAX_URI_SIZE+CACHE_PATH_LEN] = {0};
   char uri[MAX_URI_SIZE] = {0};

   if (strlen(requestURI) < MAX_URI_SIZE) {
      scannedFields = sscanf(requestURI, "/X%Lf_Y%Lf_Z%d.%3s", &center.x, &center.y, &zoom, extension);

      if (scannedFields == 4) {
         isValidURI = true;
      }
   }

   if (isValidURI) {
      strncpy(uri, requestURI, MAX_URI_SIZE);
      sprintf(cachePath, "%s%s", CACHE_PATH, uri);

      cacheFile = fopen(cachePath, "rb");

      if (cacheFile != NULL) {
         // read from cache/overlays
         // this is to ensure that overlays are not overwritten
         // the front-end server (e.g. nginx) should be handling the file serving

         imageBuffer = malloc(sizeof(unsigned char) * MAX_FILE_SIZE);
         *length = fread(imageBuffer, 1, MAX_FILE_SIZE, cacheFile);

         fclose(cacheFile);

      } else {
         // TODO: check if the flipped version (x, -y) is cached,
         // in which case the image can be loaded, flipped and served
         // since mandelbrot set is symmetrical about the real axis
         
         fractal = createMandelbrotSet(width, height, zoom, center);
         setMandelbrotSetColorPalette(fractal, (color *)colorPalette);
         generateMandelbrotSetFast(fractal);

         if (strncmp(extension, "bmp", EXT_LENGTH) == 0) {
            imageFormat = "BMP3";
         } else if (
            (strncmp(extension, "gif", EXT_LENGTH) == 0) ||
            (strncmp(extension, "png", EXT_LENGTH) == 0) ||
            (strncmp(extension, "jpg", EXT_LENGTH) == 0)
            ){
            imageFormat = extension;
         } else {
            imageFormat = "png";
         }

         imageBuffer = getMandelbrotSetImageBuffer(fractal, imageFormat, length);
         freeMandelbrotSet(fractal);

         overlayHiddenImages(overlays, center.x, center.y, zoom, width, height, &imageBuffer, length);
      }
   }

   return imageBuffer;
}
