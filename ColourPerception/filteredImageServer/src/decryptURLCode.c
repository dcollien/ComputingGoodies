#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define KEY_BITS 256
#define BYTE_SIZE 8
#define BYTE_MASK ((1 << BYTE_SIZE) - 1)
#define KEY_LENGTH (KEY_BITS/BYTE_SIZE)
#define IV_LENGTH AES_BLOCK_SIZE
#define TIMEOUT_SECONDS 1024
#define PREFIX "OKAY"
#define PREFIX_LENGTH 4
#define BUFFER_SIZE 32

#include "decryptURLCode.h"

static byte *base64Decode(byte *b64message);
static int calcDecodeLength(const byte *b64input);

bool decryptURLCode(byte *base64Input, byte *keyBytes, byte *decryptedText) {
	uint32_t now;
	int timestamp;
	int timestampExtra;
	int adjacentTimestamp;
	int i;
	bool isCorrectlyDecrypted;
	
	byte key[KEY_LENGTH];
	byte ivNow[IV_LENGTH];
	byte ivAdjacent[IV_LENGTH];

	byte *cipherBytes;
	byte decryptedBytes[AES_BLOCK_SIZE];
	byte *hexWritingPtr;
	AES_KEY decryptKey;

	// convert url-safe base64 to standard base64
	i = 0;
	while (i != strlen((char *)base64Input)) {
		if (base64Input[i] == '_') {
			base64Input[i] = '/';
		} else if (base64Input[i] == '-') {
			base64Input[i] = '+';
		}

		++i;
	}

	cipherBytes = base64Decode(base64Input);

	memcpy(key, keyBytes, KEY_LENGTH);

	now = time(NULL);
	timestamp = now / TIMEOUT_SECONDS;
	timestampExtra = now % TIMEOUT_SECONDS;

	if (timestampExtra > TIMEOUT_SECONDS/2) {
		adjacentTimestamp = timestamp + 1;
	} else {
		adjacentTimestamp = timestamp - 1;
	}

	memset(ivNow, 0, IV_LENGTH);
	memset(ivAdjacent, 0, IV_LENGTH);

	// put timestamps into IVs
	i = 0;
	while (timestamp > 0) {
		ivNow[i] = timestamp & BYTE_MASK;
		timestamp >>= BYTE_SIZE;
		++i;
	}

	i = 0;
	while (timestamp > 0) {
		ivAdjacent[i] = adjacentTimestamp & BYTE_MASK;
		adjacentTimestamp >>= BYTE_SIZE;
		++i;
	}

	// decrypt as one block
	AES_set_decrypt_key(key, KEY_BITS, &decryptKey);
	AES_cbc_encrypt(cipherBytes, decryptedBytes, AES_BLOCK_SIZE, &decryptKey, ivNow, AES_DECRYPT);

	isCorrectlyDecrypted = (strncmp((char *)decryptedBytes, PREFIX, PREFIX_LENGTH) == 0);

	if (!isCorrectlyDecrypted) {
		AES_cbc_encrypt(cipherBytes, decryptedBytes, AES_BLOCK_SIZE, &decryptKey, ivAdjacent, AES_DECRYPT);
		isCorrectlyDecrypted = (strncmp((char *)decryptedBytes, PREFIX, PREFIX_LENGTH) == 0);
	}

	// clean up
	free(cipherBytes);

	// convert to hex string
	hexWritingPtr = decryptedText;
	i = PREFIX_LENGTH; // skip prefix
	while (i != AES_BLOCK_SIZE) {
		hexWritingPtr += sprintf((char *)hexWritingPtr, "%02x", decryptedBytes[i]);
		++i;
	};
	*hexWritingPtr = '\0';

	// override key with zeros, no peeking!
	memset(key, 0, KEY_LENGTH);

	return isCorrectlyDecrypted;
}

// Calculates the length of a decoded base64 string
static int calcDecodeLength(const byte *b64input) { 
	int len = strlen((char *)b64input);
	int padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') {
		padding = 2;
	} else if (b64input[len-1] == '=') {
		padding = 1;
	}

	return (int)len*0.75 - padding;
}

// Decodes a base64 encoded string
static byte *base64Decode(byte *b64message) {
	BIO *bmem, *b64;
	int decodeLen = calcDecodeLength(b64message);
	int len = 0;

	byte *buffer = (byte *)malloc(decodeLen+1);

	b64 = BIO_new(BIO_f_base64());
	// Do not use newlines to flush buffer
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf(b64message, strlen((char *)b64message));
  	bmem = BIO_push(b64, bmem);
	len = BIO_read(bmem, buffer, decodeLen);

	assert(len == decodeLen);
	buffer[len] = '\0';

	BIO_free_all(bmem);

	return buffer;
}
