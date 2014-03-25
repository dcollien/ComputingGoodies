// given a URI, image size and file extension return a buffer and length for an image
// either generated or grabbed from the image cache
unsigned char *servePerceptionImage (char *requestURI, char *extension, size_t *length);

// initialise
void init_perception (const char *username, const char *password);
void deinit_perception (void);

// store an image buffer to the file cache
void saveToCache (char *requestURI, unsigned char *imageBuffer, int length);
