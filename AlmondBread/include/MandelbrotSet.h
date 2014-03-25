#define DEFAULT_MAX_ITERATIONS 255

#define USE_IMAGE_PROCESSING

typedef struct mandelbrotSetData *MandelbrotSet;

typedef long double real;

typedef struct {
   real x;
   real y;
} mandelbrotCoord;

typedef struct {
   double red;
   double green;
   double blue;
} color;

MandelbrotSet createMandelbrotSet(int width, int height, int zoom, mandelbrotCoord center);
void freeMandelbrotSet(MandelbrotSet fractal);

void generateMandelbrotSet(MandelbrotSet fractal);
void generateMandelbrotSetFast(MandelbrotSet fractal);

#ifdef USE_IMAGE_PROCESSING
// returns a newly allocated image buffer, requires freeing
unsigned char *getMandelbrotSetImageBuffer(MandelbrotSet fractal, const char *format, size_t *length);

void setMandelbrotSetColorPalette(MandelbrotSet fractal, color *colorPalette);
#endif

// returns a borrowed reference (freed when fractal is freed)
// indexed by [row][col]
int **getMandelbrotSetScores(MandelbrotSet fractal);

void setMandelbrotSetMaxIterations(MandelbrotSet fractal, int maxIterations);
