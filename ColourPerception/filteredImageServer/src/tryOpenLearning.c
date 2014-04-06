#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "OpenLearning.h"

int main (int argc, char *argv[]) {
	assert (argc == 3);

	char *user = argv[1];
	char *password = argv[2];
	char *output;

	char cohortPath[1024];
	char userID[1024];

	OLConnection ol = new_OLConnection (user, password);

	// Insecure: only for debugging use
	printf("Cohort: ");
	scanf("%s", cohortPath);
	printf("User: ");
	scanf("%s", userID);

	JSON object = get_OLConnection_userCohortInfo (ol, cohortPath, userID);
	output = json_dumps (object, JSON_INDENT(2));

	printf("%s\n", output);

	free(output);
	json_decref (object);
	dispose_OLConnection (ol);

	return EXIT_SUCCESS;
}
