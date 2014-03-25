#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <event.h>
#include <evhttp.h>

#include <signal.h>

#include "imageInit.h"

#include "mandelbrotServer.h"

#define BUFFER_SIZE 8192

#define EXT_LENGTH 3

#define WIDTH 512
#define HEIGHT 512

#define DEFAULT_COLOR_MAP "../colormaps/default.ColorMap"

void brokenPipe(int signum);
void addHeaders(struct evhttp_request *request, char *extension, unsigned int length);
void mandelbrotRequestHandler(struct evhttp_request *request, void *arg);

int main(int argc, char **argv) {
   short          http_port = 8082;
   char          *http_addr = "127.0.0.1";
   struct evhttp *http_server = NULL;

   // don't exit on broken pipe (just fail with message)
   signal(SIGPIPE, brokenPipe);

   FILE *colorMap;

   // open color map and init mandelbrot
   if (argc > 1 && (colorMap = fopen(argv[1], "r")) != NULL) {
      initMandelbrot(colorMap);
   } else {
      colorMap = fopen(DEFAULT_COLOR_MAP, "r");

      if (colorMap == NULL) {
         fprintf(stderr, "Unable to open " DEFAULT_COLOR_MAP "\n");
         exit(EXIT_FAILURE);
      }
      
      initMandelbrot(colorMap);
   }
   fclose(colorMap);
   
   // initialise image library
   init_imageTools();

   // start the eventlib http server
   event_init();
   http_server = evhttp_start(http_addr, http_port);
   evhttp_set_gencb(http_server, mandelbrotRequestHandler, NULL);

   fprintf(stderr, "Server started on port %d\n", http_port);
   event_dispatch();

   // uninitialise image library
   deinit_imageTools();

   deinitMandelbrot();

   fprintf(stderr, "Died\n");

   return EXIT_SUCCESS;
}

void brokenPipe(int signum) {
   fprintf(stderr, "Broken Pipe\n");
}

void addHeaders(struct evhttp_request *request, char *extension, unsigned int length) {
   char headerValue[BUFFER_SIZE] = {0};

   evhttp_add_header(request->output_headers, "Server", "AlmondBread");

   if (strncmp(extension, "bmp", EXT_LENGTH) == 0) {
      evhttp_add_header(request->output_headers, "Content-Type", "image/bmp");
   } else if (strncmp(extension, "png", EXT_LENGTH) == 0) {
      evhttp_add_header(request->output_headers, "Content-Type", "image/png");
   } else if (strncmp(extension, "jpg", EXT_LENGTH) == 0) {
      evhttp_add_header(request->output_headers, "Content-Type", "image/jpeg");
   } else if (strncmp(extension, "gif", EXT_LENGTH) == 0) {
      evhttp_add_header(request->output_headers, "Content-Type", "image/gif");
   } else {
      evhttp_add_header(request->output_headers, "Content-Type", "application/octet-stream");
   }

   memset(headerValue, 0, strlen(headerValue));
   sprintf(headerValue, "%d", length);

   evhttp_add_header(request->output_headers, "Content-Length", headerValue);
}

void mandelbrotRequestHandler(struct evhttp_request *request, void *arg) {
   fprintf(stderr, "%s\n", request->uri);

   size_t length = 0;
   unsigned char *imageBuffer;

   char extension[EXT_LENGTH+1] = "png";

   struct evbuffer *responseBuffer = evbuffer_new();

   imageBuffer = serveMandelbrot(request->uri, WIDTH, HEIGHT, extension, &length);
   
   if (imageBuffer == NULL) {
      evbuffer_add_printf(responseBuffer, "Unknown format (use /X(real)_Y(real)_Z(integer).png, e.g. /X-0.5_Y0.0_Z8.png)");
      evhttp_send_reply(request, HTTP_NOTFOUND, "Not Found", responseBuffer);
   } else {
      addHeaders(request, extension, (unsigned int)length);

      evbuffer_add(responseBuffer, imageBuffer, length);
      evhttp_send_reply(request, HTTP_OK, "", responseBuffer);

      saveToCache(request->uri, imageBuffer, length);

      free(imageBuffer);
   }

   evbuffer_free(responseBuffer);
   return;
}

