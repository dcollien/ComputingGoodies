#include <stdlib.h>
#include <string.h>

#define HASH_OFFSET 2166136261
#define HASH_PRIME 16777619
#define HASH_MASK 0xFFFFFFFF

// Weak string signing
static unsigned int stringHash (const char *string, int secret);

char *encodeString (const char *string, unsigned int secret, int *length) {
	static char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	static int alphabetSize = 36;

	unsigned int hash = stringHash (string, secret);

	char *encoded = malloc (1);
	int stringSize = 1;

	*length = 0;
	while (hash > 0) {
		if (stringSize <= (*length)) {
			stringSize *= 2;
			encoded = realloc (encoded, stringSize);
		}
		encoded[*length] = alphabet[(hash % alphabetSize)];
		(*length)++;
		hash /= alphabetSize;
	}

	encoded[*length] = '\0';

	return encoded;
}

static unsigned int stringHash (const char *string, int secret) {
	int numChars = strlen (string);
	int i;
	unsigned int hash = HASH_OFFSET;

	for (i = 0; i != numChars; ++i) {
		hash ^= string[i];
		hash *= HASH_PRIME;
		hash &= HASH_MASK;
	}

	hash ^= secret;
	hash &= HASH_MASK;

	return hash;
}

