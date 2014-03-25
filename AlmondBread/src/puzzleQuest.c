#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <dirent.h>
#include <assert.h>

#include "puzzleQuest.h"
#include "imageOverlay.h"

#define PATH_SIZE 1024

#define MAX_FILE_SIZE 5242880

#define OVERLAY_WIDTH 512
#define OVERLAY_HEIGHT 512

void destroyOverlays(overlay_t *overlay) {
   overlay_t *next;

   while(overlay != NULL) {
      next = overlay->next;
      free(overlay->bytes);
      free(overlay);
      overlay = next;
   }
}

overlay_t *initOverlays(const char *overlayPath) {
   overlay_t *firstOverlay = NULL;
   overlay_t *prevOverlay = NULL;
   overlay_t *overlay = NULL;

   FILE *overlayFile = NULL;
   char filePath[PATH_SIZE] = "";
   char extension[4] = "xxx";

   int scannedFields = 0;

   DIR *dir;
   struct dirent *ent;

   dir = opendir(overlayPath);
   
   assert(dir != NULL);

   // list contents of directory
   while ((ent = readdir(dir)) != NULL) {
      if (ent->d_name[0] == '.') {
         continue;
      }

      printf("Loading overlay: %s\n", ent->d_name);

      overlay = malloc(sizeof(struct overlay));
      overlay->next = NULL;

      if (prevOverlay != NULL) {
         prevOverlay->next = overlay;
      }

      scannedFields = sscanf(ent->d_name, "X%Lf_Y%Lf_Z%d.%3s", &overlay->x, &overlay->y, &overlay->zoom, extension);
      assert(scannedFields == 4);

      strncpy(filePath, overlayPath, PATH_SIZE);
      strncat(filePath, "/", PATH_SIZE);
      strncat(filePath, ent->d_name, PATH_SIZE);

      overlayFile = fopen(filePath, "rb");

      assert(overlayFile != NULL);

      overlay->bytes = malloc(sizeof(unsigned char) * MAX_FILE_SIZE);
      overlay->length = fread(overlay->bytes, 1, MAX_FILE_SIZE, overlayFile);

      if (firstOverlay == NULL) {
         firstOverlay = overlay;
      }

      fclose(overlayFile);
   }
   closedir (dir);

   return firstOverlay;
}

void overlayHiddenImages(
   overlay_t *overlays,

   long double x,
   long double y,
   int zoom,
   int imageWidth,
   int imageHeight,

   unsigned char **imageBufferPtr,
   size_t *length
   ) {
   
   unsigned char *imageBuffer = *imageBufferPtr;
   overlay_t *overlay = overlays;

   unsigned long long invResolution = ((unsigned long long)1) << zoom;

   double scale = 1.0;
   int overlayCol = 0;
   int overlayRow = 0;

   int offsetX = 0;
   int offsetY = 0;

   while (overlay != NULL) {
      overlayCol = (x - overlay->x) * invResolution;
      overlayRow = (y - overlay->y) * invResolution;
      
      printf("Overlay: scale: %lf, overlay: (%Lf, %Lf), image: (%Lf, %Lf), pixel offset: (%d, %d)\n", 
            scale,
            overlay->x, overlay->y, 
            x, y,
            overlayCol, overlayRow);

      if (overlayCol >= -imageWidth/2 && overlayCol <= imageWidth/2 && overlayRow >= -imageHeight/2 && overlayRow <= imageHeight/2) {
         // on screen
         scale = pow(2, zoom-(overlay->zoom));

         offsetX = imageWidth/2 - overlayCol - (scale * OVERLAY_WIDTH/2);
         offsetY = imageHeight/2 + overlayRow - (scale * OVERLAY_HEIGHT/2);

         printf("On Screen: %d, %d \n", offsetX, offsetY);

         if (scale * OVERLAY_WIDTH > 1) {
            *imageBufferPtr = get_overlayedImage(
               // input params
               imageBuffer,
               *length,

               overlay->bytes,
               overlay->length,

               offsetX,
               offsetY,

               scale,

               // output param
               length
            );

            free(imageBuffer);
            imageBuffer = *imageBufferPtr;
         }
      }

      overlay = overlay->next;
   }
}