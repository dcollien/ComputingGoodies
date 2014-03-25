#define EXT_LENGTH 3
#define MAX_URI_SIZE 512

// given a URI, image size and file extension return a buffer and length for a mandelbrot image
// either generated or grabbed from the image cache
unsigned char *serveMandelbrot(char *requestURI, int width, int height, char *extension, size_t *length);

// initialise with a color map file
void initMandelbrot(FILE *colorMapFile);

// clean up allocated memory
void deinitMandelbrot(void);

// store an image buffer to the file cache
void saveToCache(char *requestURI, unsigned char *imageBuffer, int length);
