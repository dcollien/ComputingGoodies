#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "MandelbrotSet.h"

#ifdef USE_IMAGE_PROCESSING
#include "imageGenerate.h"
#endif

#define ESCAPE_RADIUS_SQ 4

struct mandelbrotSetData {
   int width;
   int height;
   mandelbrotCoord center;
   
   real resolution;
   real top;
   real left;

   int **pixelScores;
   color *colorPalette;
   int maxIterations;
};

static void generateSetPortion(MandelbrotSet fractal, int startRow, int startCol, int endRow, int endCol);
static void allocatePixelMemory(MandelbrotSet fractal);

static inline int  escapeScore(MandelbrotSet fractal, mandelbrotCoord coord);
static inline void generateSetPixel(MandelbrotSet fractal, int row, int col);

#ifdef USE_IMAGE_PROCESSING
static void mandelbrotPixelGenerator(int x, int y, double *red, double *green, double *blue, MandelbrotSet fractal);
#endif

static void generateDivideAndConquer(MandelbrotSet fractal, int startX, int startY, int width, int height);

static inline bool generateBlockRow(MandelbrotSet fractal, int row, int colStart, int width);
static inline bool generateBlockCol(MandelbrotSet fractal, int col, int rowStart, int height);


MandelbrotSet createMandelbrotSet(int width, int height, int zoom, mandelbrotCoord center) {
   MandelbrotSet fractal = malloc(sizeof (struct mandelbrotSetData));

   fractal->width  = width;
   fractal->height = height;
   fractal->center = center;
   
   // calculate the mandelbrot-space distance between pixels
   fractal->resolution = 1.0/((real)((unsigned long long)1 << zoom));

   // width and height in fractal coordinates (from image coordinates)
   real fractalWidth  = width  * fractal->resolution;
   real fractalHeight = height * fractal->resolution;

   // top-left coordinate of the viewport rectangle in fractal coordinates
   fractal->left = center.x - (fractalWidth/2.0);
   fractal->top  = center.y + (fractalHeight/2.0);

   // to be allocated later
   fractal->pixelScores  = NULL;
   fractal->colorPalette = NULL;

   fractal->maxIterations = DEFAULT_MAX_ITERATIONS;

   return fractal;
}

void freeMandelbrotSet(MandelbrotSet fractal) {
   int row;
   if (fractal->pixelScores != NULL) {
      for (row = 0; row != fractal->height; ++row) {
         free(fractal->pixelScores[row]);
      }
      free(fractal->pixelScores);
   }
   free(fractal);
}

void generateMandelbrotSet(MandelbrotSet fractal) {
   allocatePixelMemory(fractal);
   generateSetPortion(fractal, 0, 0, fractal->width, fractal->height);
}

void generateMandelbrotSetFast(MandelbrotSet fractal) {
   allocatePixelMemory(fractal);
   generateDivideAndConquer(fractal, 0, 0, fractal->width, fractal->height);
}

#ifdef USE_IMAGE_PROCESSING
// returns a new image buffer, requires freeing
unsigned char *getMandelbrotSetImageBuffer(MandelbrotSet fractal, const char *format, size_t *length) {
   assert(fractal->pixelScores != NULL);
   pixelOperator pixelGen = (pixelOperator)&mandelbrotPixelGenerator;
   return get_imageGeneration(format, fractal->width, fractal->height, pixelGen, fractal, length);
}


void setMandelbrotSetColorPalette(MandelbrotSet fractal, color *colorPalette) {
   fractal->colorPalette = colorPalette;
}
#endif

// returns a borrowed reference (freed when fractal is freed)
int **getMandelbrotSetScores(MandelbrotSet fractal) {
   return fractal->pixelScores;
}

void setMandelbrotSetMaxIterations(MandelbrotSet fractal, int maxIterations) {
   fractal->maxIterations = maxIterations;
}

static void allocatePixelMemory(MandelbrotSet fractal) {
   int row;
   if (fractal->pixelScores == NULL) {
      // only need to allocate if not yet allocated

      fractal->pixelScores = (int **)malloc(sizeof(int*) * fractal->height);
      for (row = 0; row != fractal->height; ++row) {
         fractal->pixelScores[row] = (int *)malloc(sizeof(int) * fractal->width);
      }
   }
}

static void generateSetPortion(MandelbrotSet fractal, int startX, int startY, int width, int height) {
   int row, col;

   for (row = startY; row != startY+height; ++row) {
      for (col = startX; col != startX+width; ++col) {
         generateSetPixel(fractal, row, col);
      }
   }
}

// TODO: consider implementing circle tiling http://mrob.com/pub/muency/circletiling.html

static void generateDivideAndConquer(MandelbrotSet fractal, int startX, int startY, int width, int height) {
   // Mariani/Silver algorithm http://mrob.com/pub/muency/marianisilveralgorithm.html
   // Warning: may miss cusps narrower than 1 pixel

   int row, col;
   bool canSkip;
   
   int firstRow = startY;
   int lastRow  = startY + height - 1;
   int firstCol = startX;
   int lastCol  = startX + width - 1;

   int newWidth, newHeight;

   if (width < 3 || height < 3) {
      // stopping case, generate the normal way
      generateSetPortion(fractal, startX, startY, width, height);
   } else {
      // check top and bottom most rows
      canSkip = generateBlockRow(fractal, firstRow, firstCol, width);
      canSkip = canSkip && generateBlockRow(fractal, lastRow, firstCol, width);

      // check left and right most columns (not including top and bottom rows)
      canSkip = canSkip && generateBlockCol(fractal, firstCol, firstRow+1, height-2);
      canSkip = canSkip && generateBlockCol(fractal, lastCol, firstRow+1, height-2);

      if (canSkip && fractal->pixelScores[firstRow][firstCol] > 1) {
         // pruning case

         // this block is entirely bordered by the same score, which isn't 0
         // fill the rest in with this score
         for (row = firstRow+1; row != firstRow+1+height-2; ++row) {
            for (col = firstCol+1; col != firstCol+1+width-2; ++col) {
               fractal->pixelScores[row][col] = fractal->pixelScores[firstRow][firstCol];
            }
         }
      } else {
         // recursive case
         newWidth = width/2;
         newHeight = height/2;

         // split and generate 4 quadrants
         generateDivideAndConquer(fractal, startX, startY, newWidth, newHeight);
         generateDivideAndConquer(fractal, startX+newWidth, startY, width-newWidth, newHeight);
         generateDivideAndConquer(fractal, startX, startY+newHeight, newWidth, height-newHeight);
         generateDivideAndConquer(fractal, startX+newWidth, startY+newHeight, width-newWidth, height-newHeight);
      }
   }
}

static inline bool generateBlockRow(MandelbrotSet fractal, int row, int colStart, int width) {
   bool isSameColor = true;
   int col = colStart;
   while (isSameColor && col != colStart+width) {
      generateSetPixel(fractal, row, col);
      if (col > 0 && fractal->pixelScores[row][col] != fractal->pixelScores[row][col-1]) {
         isSameColor = false;
      }
      col++;
   }

   return isSameColor;
}

static inline bool generateBlockCol(MandelbrotSet fractal, int col, int rowStart, int height) {
   bool isSameColor = true;
   int row = rowStart;
   while (isSameColor && row != rowStart+height) {
      generateSetPixel(fractal, row, col);
      if (row > 0 && fractal->pixelScores[row][col] != fractal->pixelScores[row-1][col]) {
         isSameColor = false;
      }
      row++;
   }

   return isSameColor; 
}

static inline void generateSetPixel(MandelbrotSet fractal, int row, int col) {
   mandelbrotCoord coord;
   real halfResolution = fractal->resolution/2.0;

   // generate coordinate, shift to the center of the pixel
   coord.x = fractal->left + (fractal->resolution * col + halfResolution);
   coord.y = fractal->top - (fractal->resolution * row + halfResolution);

   fractal->pixelScores[row][col] = escapeScore(fractal, coord);
}

static inline int escapeScore(MandelbrotSet fractal, mandelbrotCoord coord) {
   int score;
   real xSq, ySq;
   real tempX, x, y;

   // check for a known exit 
   // (using a polynomial approximation of the large inside area of the set)
   real xShifted = (coord.x - 0.25);
   real sqCoordY = (coord.y*coord.y);
   real q = xShifted*xShifted + sqCoordY;
   if ((q * (q + xShifted)) < (0.25 * sqCoordY)) {
      // confirmed inside the set
      score = fractal->maxIterations;
   } else {
      // unknown, calculate it slowly

      // TODO: add orbit detection: http://mrob.com/pub/muency/orbitdetection.html
      // using Floyd's tortoise/hare cycle detection

      score = 0;
      x = 0;
      y = 0;
      xSq = 0;
      ySq = 0;
      while (xSq + ySq < ESCAPE_RADIUS_SQ && score != fractal->maxIterations) {
         tempX = xSq - ySq + coord.x;
         y = 2*x*y + coord.y;
         x = tempX;

         xSq = x*x;
         ySq = y*y;
         score++;
      } 
   }

   return score;
}

#ifdef USE_IMAGE_PROCESSING
static void mandelbrotPixelGenerator(int x, int y, double *red, double *green, double *blue, MandelbrotSet fractal) {
   double value;
   int score = fractal->pixelScores[y][x];
   
   if (fractal->colorPalette == NULL) {
      value = score/fractal->maxIterations;
      *red = value;
      *green = value;
      *blue = value;
   } else {
      *red = fractal->colorPalette[score].red;
      *green = fractal->colorPalette[score].green;
      *blue = fractal->colorPalette[score].blue;
   }
}
#endif

