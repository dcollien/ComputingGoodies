#ifndef OPEN_LEARNING_H
#define OPEN_LEARNING_H

// http://www.digip.org/jansson/doc/2.4/apiref.html#decoding
#include <jansson.h>
#include <stdbool.h>

typedef struct OL_data *OLConnection;
typedef json_t *JSON;

OLConnection new_OLConnection (const char *username, const char *password);

JSON get_OLConnection_coursesWithAttribute (OLConnection connection, const char *attribute);
JSON get_OLConnection_enrolments (OLConnection connection, const char *coursePath);
JSON get_OLConnection_userInfo (OLConnection connection, const char *profileName);
JSON get_OLConnection_userCohortInfo (OLConnection connection, const char *cohortPath, const char *profileName);
JSON get_OLConnection_cohortUserData (OLConnection connection, const char *cohortPath, const char *profileName, const char *key);

JSON set_OLConnection_cohortUserData (OLConnection connection, const char *cohortPath, const char *profileName, const char *key, const char *value);

void dispose_OLConnection (OLConnection connection);

#endif
