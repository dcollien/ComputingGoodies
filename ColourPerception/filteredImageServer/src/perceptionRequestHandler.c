#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdbool.h>
#include <libgen.h>
#include <errno.h>

#include "OpenLearning.h"

// transforming pixels in images
#include "imageTransform.h"
#include "imageOverlay.h"

// how to transform the image for perception
#include "perceptionTransformer.h"

// user's vision to transform the image
#include "vision.h"


#include "perceptionRequestHandler.h"
#include "weakEncrypt.h"

#define BUF_SIZE 1024

#define MAX_FILE_SIZE 5242880

#define EXPECTED_FIELDS_ZOOMING 4
#define EXPECTED_FIELDS_FILE 1

#define EXTENSION_LENGTH 4

#define SECRET 0x902DF34D

#define COLOR_FIELD_NAME "What is your favourite colour?"


#define CACHE_PATH "./imageCache"
#define CACHE_PATH_LEN 12

#define REPO_PATH "./images"

#define REPO_FILE "./imageList.txt"

#define DEFAULT_X 250
#define DEFAULT_Y 187.5
#define DEFAULT_Z 0

static char **fileNames;
static const char *username;
static const char *password;
static int numFilenames;

static bool isValidFilename (const char *fileName) {
   int i;

   for (i = 0; i != numFilenames; ++i) {
      fprintf(stderr, "%s\n", fileNames[i]);
      if (strncmp(fileNames[i], fileName, BUF_SIZE) == 0) {
         return true;
      }
   }

   return false;
};

static void mkpath (const char *dir) {
   char tmp[256];
   char *p = NULL;
   size_t len;

   snprintf(tmp, sizeof(tmp), "%s", dir);
   len = strlen(tmp);
   if(tmp[len - 1] == '/') {
      tmp[len - 1] = 0;
   }

   for(p = tmp + 1; *p; p++) {
      if(*p == '/') {
         *p = 0;
         mkdir(tmp, S_IRWXU|S_IRWXG|S_IXOTH|S_IROTH);
         *p = '/';
      }
   }

   mkdir(tmp, S_IRWXU|S_IRWXG|S_IXOTH|S_IROTH);
}

static unsigned char *getZoomedImage (
   unsigned char *inputFileBuffer, size_t inputFileLength, 
   long double centerX, long double centerY, int zoom, 
   size_t *outputLength) {
   unsigned char *imageBuffer = NULL;

   unsigned long long scale = ((unsigned long long)1) << zoom;
   int col = centerX * scale;
   int row = centerY * scale;

   int imageWidth = 0;
   int imageHeight = 0;

   int offsetX = 0; 
   int offsetY = 0; 

   get_imageDimensions(inputFileBuffer, inputFileLength, &imageWidth, &imageHeight);

   offsetX = -col - imageWidth/2 * scale;
   offsetY = row - imageHeight/2 * scale;

   imageBuffer = get_overlayedImage(
      // no background
      NULL,
      0,

      // scaled image
      inputFileBuffer,
      inputFileLength,

      offsetX,
      offsetY,

      scale,

      // output param
      outputLength
   );

   return imageBuffer;
}

// given a URI, image size and file extension return a buffer and length for an image
// either generated or grabbed from the image cache
unsigned char *servePerceptionImage (char *requestURI, char *extension, size_t *length) {
   unsigned char *imageBuffer;

   //char *imageFormat = NULL;

   char profileName[BUF_SIZE] = "";
   char profileHash[BUF_SIZE] = "";
   char cohortPath[BUF_SIZE] = "";
   char fileName[BUF_SIZE] = "";
   char filePath[BUF_SIZE] = "";

   int scannedFields;

   long double centerX = 0.0;
   long double centerY = 0.0;
   int zoom = 0;

   bool isZooming = true;

   int encodingLength;

   unsigned char inputFileBuffer[MAX_FILE_SIZE];
   size_t inputFileLength;

   unsigned char *untransformedImageBuffer = NULL;
   unsigned char *zoomedImageBuffer = NULL;
   size_t untransformedImageBufferLength = 0;

   int karma = 0;
   int completedActivities = 0;
   int favColor = COLOR_RED;

   vision colorVision;

   FILE *imageFile;

   JSON json_userInfoObject;
   JSON json_karmaObject;
   JSON json_karma;
   JSON json_progress;
   JSON json_completedActivities;
   JSON json_extraData;
   JSON json_favColor;

   int i;

   char *token;
   char *uriCopy;
   char *toFree;

   // create a new connection to OpenLearning (given username, password)
   OLConnection ol = new_OLConnection (username, password);

   char *encodedProfileName;
   if (strlen (requestURI) >= BUF_SIZE) {
      // uri too long
      fprintf(stderr, "Request URI too long\n");
      return NULL;
   }
   
   // +1 to skip first '/'
   toFree = uriCopy = strndup (requestURI+1, BUF_SIZE);
   
   token = strsep (&uriCopy, "/");
   if (token == NULL) {
      return NULL;
   }
   strncpy (cohortPath, token, BUF_SIZE);
   fprintf (stderr, "Cohort Data: %s\n", cohortPath);

   token = strsep (&uriCopy, "/");
   if (token == NULL) {
      return NULL;
   }
   strncpy (profileName, token, BUF_SIZE);
   fprintf (stderr, "Profile Name: %s\n", profileName);

   token = strsep (&uriCopy, "/");
   if (token == NULL) {
      return NULL;
   }
   strncpy (profileHash, token, BUF_SIZE);
   fprintf (stderr, "Profile Hash: %s\n", profileHash);
   if (token == NULL) {
      return NULL;
   }

   token = strsep (&uriCopy, "/");
   strncpy (fileName, token, BUF_SIZE);
   if (token == NULL) {
      return NULL;
   }
   fprintf (stderr, "File Name: %s\n", fileName);

   if (uriCopy == NULL) {
      return NULL;
   }
   fprintf (stderr, "Remaining Path: %s\n", uriCopy);


   scannedFields = sscanf (uriCopy, "X%Lf_Y%Lf_Z%d.%3s", &centerX, &centerY, &zoom, extension);

   if (scannedFields != EXPECTED_FIELDS_ZOOMING) {

      scannedFields = sscanf (uriCopy, "image.%3s", extension);

      if (scannedFields != EXPECTED_FIELDS_FILE) {
         // malformed uri

         fprintf (stderr, "Request URI malformed, received %d fields\n", scannedFields);
         return NULL;
      } else {
         isZooming = false;
      }
   }

   free (toFree);

   for (i = 0; i != strlen (cohortPath); ++i) {
      if (cohortPath[i] == '-') {
         cohortPath[i] = '/';
      }
   }

   fprintf (stderr, "Extension: %s\n", extension);
   
   fprintf (stderr, "Cohort Path: %s\n", cohortPath);


   sprintf (fileName, "%s.%s", fileName, extension);

   fprintf (stderr, "Filename: %s\n", fileName);


   encodedProfileName = encodeString (profileName, SECRET, &encodingLength);


   if (strncmp (encodedProfileName, profileHash, encodingLength) != 0) {
      // access denied

      fprintf (stderr, "Access Denied %s != %s\n", encodedProfileName, profileHash);

      free (encodedProfileName);
      return NULL;
   }

   fprintf (stderr, "Authentication Passed\n");

   free (encodedProfileName);

   if (!isValidFilename (fileName)) {
      // not found


      fprintf (stderr, "File not found: %s\n", fileName);
      return NULL;
   }

   fprintf (stderr, "File Recognised\n");


   // TODO: bmp serving

   json_userInfoObject = get_OLConnection_userCohortInfo (ol, cohortPath, profileName);

   if (!json_is_object (json_userInfoObject)) {

      toFree = json_dumps (json_userInfoObject, JSON_INDENT(2));

      fprintf (stderr, "No user info object: %s\n", toFree);

      free (toFree);

      fprintf (stderr, "Using default vision values\n");
      favColor = COLOR_NONE;
   } else {

      assert (json_is_object (json_userInfoObject));

      fprintf (stderr, "OpenLearning Data Received\n");
      
      // get karma
      karma = 0;
      json_karmaObject = json_object_get (json_userInfoObject, "karma");
      if (json_is_object (json_karmaObject)) {
         json_karma = json_object_get (json_karmaObject, "comment");
         if (json_is_integer (json_karma)) {
            karma = json_integer_value (json_karma);
         }
      }

      fprintf (stderr, "Karma: %d\n", karma);

      // get extra data
      json_extraData = json_object_get (json_userInfoObject, "extraData");
      if (json_is_object (json_extraData)) {
         json_favColor = json_object_get (json_extraData, COLOR_FIELD_NAME);

         if (json_is_string (json_favColor)) {
            favColor = getColorFromString (json_string_value (json_favColor));
         } else {
            // TODO extract favColor
            favColor = COLOR_NONE;
         }
      } else {
         favColor = COLOR_NONE;
      }
      
      fprintf (stderr, "Fav Color: %d\n", favColor);
      
      // get progress
      json_progress = json_object_get (json_userInfoObject, "progress");
      if (json_is_object (json_progress)) {
         json_completedActivities = json_object_get (json_progress, "completed");
         assert (json_is_integer (json_completedActivities));

         completedActivities = json_integer_value (json_completedActivities);
      } else {
         completedActivities = 0;
      }
      
      fprintf (stderr, "Progress: %d\n", completedActivities);
   }

   if (favColor == COLOR_NONE) {
      // no color selected means no color perception
      favColor = COLOR_MAGENTA;
      karma = 0;
      completedActivities = 0;
   }

   colorVision = getVisionForUser (profileName, karma, favColor, completedActivities);

   fprintf (stderr, "Color Vision Calculated\n");

   snprintf (filePath, BUF_SIZE, "%s/%s", REPO_PATH, fileName);
   fprintf (stderr, "Opening: %s\n", filePath);
   imageFile = fopen (filePath, "r");

   if (imageFile != NULL) {
      inputFileLength = fread (inputFileBuffer, 1, MAX_FILE_SIZE, imageFile);
   } else {
      // no file, config lied

      fprintf (stderr, "File does not exist: %s\n", fileName);
      return NULL;
   }

   if (ferror (imageFile)) {
      fprintf (stderr, "Unable to Read File: %s\n", strerror (ferror (imageFile)));
      return NULL;
   }

   fprintf (stderr, "File Loaded: %d bytes\n", (int)inputFileLength);

   fclose (imageFile);

   if (isZooming && zoom > 0) {
      fprintf (stderr, "Zooming...\n");
      zoomedImageBuffer = getZoomedImage (inputFileBuffer, inputFileLength, centerX-DEFAULT_X, centerY-DEFAULT_Y, zoom-DEFAULT_Z, &untransformedImageBufferLength);
      untransformedImageBuffer = zoomedImageBuffer;
      assert (zoomedImageBuffer != NULL);
   } else {
      untransformedImageBuffer = (unsigned char *)inputFileBuffer;
      untransformedImageBufferLength = inputFileLength;
   }

   imageBuffer = get_imageTransform (
      (pixelOperator)&perceptionTransformer,
      untransformedImageBuffer,
      untransformedImageBufferLength,
      length,
      (void *)&colorVision
   );

   fprintf (stderr, "Image Transformed\n");

   json_decref (json_userInfoObject);

   dispose_OLConnection (ol);

   if (zoomedImageBuffer != NULL) {
      free (zoomedImageBuffer);
   }

   return imageBuffer;
}

// initialise
void init_perception (const char *olUser, const char *olPass) {
   struct stat existenceStat;
   int fileNamesSize;
   char fileName[BUF_SIZE];

   FILE *file;

   username = olUser;
   password = olPass;

   init_imageTools ();

   if(stat (CACHE_PATH, &existenceStat) < 0 || !S_ISDIR(existenceStat.st_mode)) {
      fprintf (stderr, "Cache directory: " CACHE_PATH " does not exist\n");
      exit (EXIT_FAILURE);
   }

   if(stat (REPO_PATH, &existenceStat) < 0 || !S_ISDIR(existenceStat.st_mode)) {
      fprintf (stderr, "Repository directory: " REPO_PATH " does not exist\n");
      exit (EXIT_FAILURE);
   }


   if(stat (REPO_FILE, &existenceStat) < 0) {
      fprintf (stderr, "Images file: " REPO_FILE " does not exist\n");
      exit (EXIT_FAILURE);
   }

   fileNames = malloc (sizeof(char *));
   numFilenames = 0;
   fileNamesSize = 1;

   fprintf (stderr, "Opening images file\n");
   file = fopen (REPO_FILE, "r");

   while (fgets (fileName, sizeof fileName, file) != NULL) {
      if (fileNamesSize <= numFilenames) {
         fileNamesSize *= 2;
         fileNames = realloc (fileNames, sizeof (char *) * fileNamesSize);
      }

      fileNames[numFilenames] = malloc (sizeof (char) * BUF_SIZE);
      sscanf (fileName, "%s", fileNames[numFilenames]);
      fprintf (stderr, "Loaded %s\n", fileNames[numFilenames]);
      numFilenames++;
   }

   fclose (file);
}

void deinit_perception (void) {
   int i;

   deinit_imageTools ();

   for (i = 0; i != numFilenames; ++i) {
      free (fileNames[i]);
   }

   free (fileNames);
}

// store an image buffer to the file cache
void saveToCache (char *requestURI, unsigned char *imageBuffer, int length) {
   FILE *cacheFile;
   char cachePath[BUF_SIZE] = {0};
   char cacheImage[BUF_SIZE] = {0};

   snprintf (cachePath, BUF_SIZE, "%s%s", CACHE_PATH, requestURI);
   snprintf (cacheImage, BUF_SIZE, "%s%s", CACHE_PATH, requestURI);

   mkpath (dirname(cachePath));


   fprintf (stderr, "Created Path: %s\n", cachePath);

   fprintf (stderr, "Saving %d bytes to cache file: %s\n", length, cacheImage);

   cacheFile = fopen (cacheImage, "wb");
   if (cacheFile == NULL) {
      fprintf (stderr, "Unable to open file for writing: %s\n", strerror (errno));
      return;
   }
   fwrite (imageBuffer, 1, length, cacheFile);
   fclose (cacheFile);
}

