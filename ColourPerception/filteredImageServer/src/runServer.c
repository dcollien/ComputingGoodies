#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <event.h>
#include <evhttp.h>

#include <signal.h>

#include "perceptionRequestHandler.h"

#define BUFFER_SIZE 8192

#define EXPECTED_ARGS 4
#define KEY_LENGTH 32

#define EXT_LENGTH 3

void brokenPipe (int signum);
void addHeaders (struct evhttp_request *request, char *extension, unsigned int length);
void perceptionRequestHandler (struct evhttp_request *request, void *arg);

int main (int argc, char **argv) {
   int keyLength;

   short          http_port = 8081;
   char          *http_addr = "127.0.0.1";
   struct evhttp *http_server = NULL;

   // don't exit on broken pipe (just fail with message)
   signal (SIGPIPE, brokenPipe);

   if (argc < EXPECTED_ARGS) {
      fprintf (stderr, "args: <OpenLearning username> <OpenLearning password> <256-bit encryption key (32 characters)>\n");
   } else {
      keyLength = strlen((char *)argv[3]);
      if (keyLength != KEY_LENGTH) {
         fprintf (stderr, "Key must be %d characters, currently %d\n", KEY_LENGTH, keyLength);
      } else {
         init_perception (argv[1], argv[2], argv[3]);
         // start the eventlib http server
         event_init ();
         http_server = evhttp_start (http_addr, http_port);
         // generic callback for http serving
         evhttp_set_gencb (http_server, perceptionRequestHandler, NULL);

         fprintf (stderr, "Server started on port %d\n", http_port);
         event_dispatch ();

         deinit_perception ();

         fprintf (stderr, "Died\n");
      }
   }

   return EXIT_SUCCESS;
}

void brokenPipe (int signum) {
   fprintf (stderr, "Broken Pipe\n");
}

void addHeaders (struct evhttp_request *request, char *extension, unsigned int length) {
   char headerValue[BUFFER_SIZE] = {0};

   if (strncmp (extension, "bmp", EXT_LENGTH) == 0) {
      evhttp_add_header (request->output_headers, "Content-Type", "image/bmp");
   } else if (strncmp (extension, "png", EXT_LENGTH) == 0) {
      evhttp_add_header (request->output_headers, "Content-Type", "image/png");
   } else if (strncmp (extension, "jpg", EXT_LENGTH) == 0) {
      evhttp_add_header (request->output_headers, "Content-Type", "image/jpeg");
   } else if (strncmp (extension, "gif", EXT_LENGTH) == 0) {
      evhttp_add_header (request->output_headers, "Content-Type", "image/gif");
   } else {
      evhttp_add_header (request->output_headers, "Content-Type", "application/octet-stream");
   }

   memset (headerValue, 0, strlen (headerValue));
   sprintf (headerValue, "%d", length);

   evhttp_add_header (request->output_headers, "Content-Length", headerValue);
}

void perceptionRequestHandler (struct evhttp_request *request, void *arg) {
   fprintf (stderr, "%s\n", request->uri);

   size_t length = 0;
   unsigned char *imageBuffer;

   char extension[EXT_LENGTH+1] = "png";

   struct evbuffer *responseBuffer = evbuffer_new ();

   imageBuffer = servePerceptionImage (request->uri, extension, &length);
   
   if (imageBuffer == NULL) {
      evbuffer_add_printf (responseBuffer, "Not Found");
      evhttp_send_reply (request, HTTP_NOTFOUND, "Not Found", responseBuffer);
   } else {
      addHeaders (request, extension, (unsigned int)length);
      
      evbuffer_add (responseBuffer, imageBuffer, length);
      evhttp_send_reply (request, HTTP_OK, "", responseBuffer);

      saveToCache (request->uri, imageBuffer, length);

      free (imageBuffer);
   }

   evbuffer_free (responseBuffer);
   return;
}

