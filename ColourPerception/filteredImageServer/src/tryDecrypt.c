#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "decryptURLCode.h"

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
	assert (argc == 3);

	byte *key = (byte *)argv[2];
	byte *base64Input = (byte *)argv[1];
	byte decryptedText[BUFFER_SIZE];

	if (decryptURLCode (base64Input, key, decryptedText)) {
		printf ("%s\n", decryptedText);
	} else {
		printf ("Invalid/Timed Out: %s\n", decryptedText);
	}
}
