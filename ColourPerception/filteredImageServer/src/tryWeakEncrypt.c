#include <stdio.h>
#include <stdlib.h>

#include "weakEncrypt.h"

int main(int argc, char *argv[]) {
	int length;
	char *encoded = encodeString(argv[1], 0x902DF34D, &length);

	printf("'%s' '%s' %d\n", argv[1], encoded, length);

	free(encoded);

	return EXIT_SUCCESS;
}
