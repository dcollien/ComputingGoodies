#include "OpenLearning.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <curl/curl.h>

#define BUF_SIZE 8192
#define MESSAGE_SIZE 2048

#define MAX_USERNAME_SIZE 256
#define MAX_PASSWORD_SIZE 256


#define RPC_PORT 80
#define RPC_HOST "https://www.openlearning.com"
#define RPC_HOST_LENGTH 28

#define TEXT_BUFFER_SIZE 1024

#define FALSE 0

struct OL_data {
   char *username;
   char *password;
};

typedef struct {
   char *memory;
   size_t size; 
} buffer_t;

static size_t writeHandler (void *contents, size_t size, size_t nmemb, void *bufferPtr);
//static size_t readHandler(void *contents, size_t size, size_t nmemb, void *bufferPtr);

static JSON connectToRPC (const char *username, const char *password, const char *path, const char *postFields);


OLConnection new_OLConnection (const char *username, const char *password) {
   OLConnection connection = malloc (sizeof(struct OL_data));
   assert (connection != NULL);

   connection->username = malloc (sizeof(char) * MAX_USERNAME_SIZE);
   connection->password = malloc (sizeof(char) * MAX_PASSWORD_SIZE);

   strncpy (connection->username, username, MAX_USERNAME_SIZE);
   strncpy (connection->password, password, MAX_PASSWORD_SIZE);

   return connection;
}

JSON get_OLConnection_coursesWithAttribute (OLConnection connection, const char *attribute) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";
   snprintf (rpcPath, TEXT_BUFFER_SIZE, "/api/listCoursesWithAttribute/%s", attribute);

   return connectToRPC (connection->username, connection->password, rpcPath, NULL);
}

JSON get_OLConnection_enrolments (OLConnection connection, const char *coursePath) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";
   snprintf(rpcPath, TEXT_BUFFER_SIZE, "/api/listEnrolments/%s", coursePath);

   return connectToRPC (connection->username, connection->password, rpcPath, NULL);
}

JSON get_OLConnection_userInfo (OLConnection connection, const char *profileName) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";
   snprintf (rpcPath, TEXT_BUFFER_SIZE, "/api/users/%s", profileName);

   return connectToRPC (connection->username, connection->password, rpcPath, NULL);
}


JSON get_OLConnection_userCohortInfo (OLConnection connection, const char *cohortPath, const char *profileName) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";
   snprintf (rpcPath, TEXT_BUFFER_SIZE, "/api/getUserCohortInfo/%s/%s", cohortPath, profileName);

   return connectToRPC (connection->username, connection->password, rpcPath, NULL);
}

JSON get_OLConnection_cohortUserData (OLConnection connection, const char *cohortPath, const char *profileName, const char *key) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";

   snprintf (rpcPath, TEXT_BUFFER_SIZE, "/api/getCohortUserData/%s/%s/%s", cohortPath, profileName, key);

   return connectToRPC (connection->username, connection->password, rpcPath, NULL);
}

JSON set_OLConnection_cohortUserData (OLConnection connection, const char *cohortPath, const char *profileName, const char *key, const char *value) {
   char rpcPath[TEXT_BUFFER_SIZE] = "";
   char postFields[TEXT_BUFFER_SIZE] = "";

   snprintf (rpcPath, TEXT_BUFFER_SIZE, "/api/setCohortUserData/%s/%s", cohortPath, profileName);
   snprintf (postFields, TEXT_BUFFER_SIZE, "key=%s&value=%s", key, value);

   return connectToRPC (connection->username, connection->password, rpcPath, postFields);
}

void dispose_OLConnection (OLConnection connection) {
   // don't leak username/password
   memset (connection->username, 0, MAX_USERNAME_SIZE);
   memset (connection->password, 0, MAX_PASSWORD_SIZE);

   free (connection->username);
   free (connection->password);
   free (connection);
}


/*
static size_t readHandler (void *outPtr, size_t size, size_t nmemb, void *bufferPtr) {
   buffer_t *buffer = (buffer_t *)bufferPtr;
   char **contents = outPtr;

   if (size*nmemb < 1) {
      return 0;
   }

   *contents = buffer->memory;
   return buffer->size;
}
*/

static size_t writeHandler (void *contents, size_t size, size_t nmemb, void *bufferPtr) {
   size_t realsize = size * nmemb;
   buffer_t *buffer = (buffer_t *)bufferPtr;

   buffer->memory = realloc (buffer->memory, buffer->size + realsize + 1);
   assert (buffer->memory != NULL);

   memcpy (&(buffer->memory[buffer->size]), contents, realsize);
   buffer->size += realsize;
   buffer->memory[buffer->size] = 0;

   return realsize;
}

static JSON connectToRPC (const char *username, const char *password, const char *path, const char *postFields) {
   JSON jsonResponse;
   CURL *curl;
   CURLcode responseCode;

   buffer_t buffer;

   json_error_t decodeError;

   int pathOffset = RPC_HOST_LENGTH;

   char address[TEXT_BUFFER_SIZE+TEXT_BUFFER_SIZE] = RPC_HOST;

   char authBuffer[MAX_USERNAME_SIZE+MAX_PASSWORD_SIZE] = "";

   strncpy (address + pathOffset, path, TEXT_BUFFER_SIZE);

   buffer.memory = malloc (1);
   buffer.size = 0;

   curl = curl_easy_init ();
   if (curl) {
      curl_easy_setopt (curl, CURLOPT_URL, address);

      curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, FALSE);
      curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, FALSE);

      curl_easy_setopt (curl, CURLOPT_HTTPAUTH,  CURLAUTH_BASIC);

      sprintf (authBuffer, "%s:%s", username, password);
      curl_easy_setopt (curl, CURLOPT_USERPWD, authBuffer);

      if (postFields != NULL) {
         curl_easy_setopt (curl, CURLOPT_POST, 1L);

         curl_easy_setopt (curl, CURLOPT_POSTFIELDS, postFields);
         //curl_easy_setopt(curl, CURLOPT_READFUNCTION, readHandler);
         //curl_easy_setopt(curl, CURLOPT_READDATA, postData);
      }

      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeHandler);
      
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *)&buffer);

      curl_easy_setopt (curl, CURLOPT_USERAGENT, "ol-api");

      responseCode = curl_easy_perform (curl);

      jsonResponse = NULL;
      if (responseCode != CURLE_OK) {
         fprintf (stderr, "Connection failed: %s\n", curl_easy_strerror (responseCode));
      } else {
         jsonResponse = json_loads (buffer.memory, JSON_DECODE_ANY, &decodeError);
      }

      curl_easy_cleanup (curl);
   } else {
      jsonResponse = NULL;
   }

   free (buffer.memory);

   // don't leak username/password
   memset (authBuffer, 0, MAX_USERNAME_SIZE+MAX_PASSWORD_SIZE);

   return jsonResponse;
}
