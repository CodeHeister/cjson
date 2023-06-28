#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <cjson/sha256.h>

#define ROT(x, shift) ((x << shift) | (x >> ((sizeof(x) * CHAR_BIT - shift) % (sizeof(x) * CHAR_BIT)));)
#define Ch(x, y, z) ((x & (y ^ z)) ^ z)
#define Maj(x, y, z)    ((x & (y | z)) | (y & z))
#define SHR(x, n)   (x >> n)
#define ROTR(x, n)  ((x >> n) | (x << (32 - n)))
#define S0(x)       (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)       (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)       (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
#define s1(x)       (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#define HASH_LENGTH 8

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static char hex_sym[16] = "0123456789abcdef";

hash_t *bytes2sha256(const char *key, size_t n) {
	if (!key)
		return NULL;

	size_t size = (n+9) * 8;
	size_t len_blk = (size-size%512)/512+1;

	char *tmp_data = (char*)malloc(len_blk*512);
	hash_t *hash = (hash_t*)malloc(sizeof(hash_t) * HASH_LENGTH);

	if (!hash || !tmp_data) {

		free(tmp_data);
		free(hash);

		tmp_data = NULL;
		hash = NULL;

		return NULL;
	}

	if (!tmp_data)
		return NULL;

	for (size_t j = 0; j < len_blk * 64; j++)
		tmp_data[j] = 0;

	{
		size_t i = 0;
		for (i = 0; key[i]; i++)
			tmp_data[i+3-(i%4)*2] = key[i];
		tmp_data[i+3-(i%4)*2] = (char)0x80;
	}

	for (size_t j = 8, tmp_size = size-72, tmp_len_blk = len_blk*64; j > 0; j--)
		tmp_data[tmp_len_blk-j+3-((tmp_len_blk-j)%4)*2] = (tmp_size >> (j-1)*8) & 0xff;

	uint32_t *data = (uint32_t*)tmp_data;

	// hash storage for calculations
	hash[0] = 0x6A09E667;
	hash[1] = 0xBB67AE85;
	hash[2] = 0x3C6EF372;
	hash[3] = 0xA54FF53A;
	hash[4] = 0x510E527F;
	hash[5] = 0x9B05688C;
	hash[6] = 0x1F83D9AB;
	hash[7] = 0x5BE0CD19;

	for (size_t i = 0; i < len_blk; i++) {

		hash_t a = hash[0];
		hash_t b = hash[1];
		hash_t c = hash[2];
		hash_t d = hash[3];
		hash_t e = hash[4];
		hash_t f = hash[5];
		hash_t g = hash[6];
		hash_t h = hash[7];

		uint32_t tmp_storage[64];

		for (size_t j = 0; j < 16; j++) 
			tmp_storage[j] = data[i*16+j];
		
		for (size_t j = 0; j < 48; j++)
			tmp_storage[j+16] = tmp_storage[j] + s0(tmp_storage[j+1]) + tmp_storage[j+9] + s1(tmp_storage[j+14]);

		for (size_t j = 0; j < 64; j++) {

			uint32_t tmp1 = h + S1(e) +	Ch(e, f, g) + tmp_storage[j] + K[j];
			uint32_t tmp2 = Maj(a, b, c) + S0(a);
			
			h = g;
			g = f;
			f = e;
			e = d + tmp1;
			d = c;
			c = b;
			b = a;
			a = tmp1 + tmp2;
		}

		hash[0] = hash[0] + a;
		hash[1] = hash[1] + b;
		hash[2] = hash[2] + c;
		hash[3] = hash[3] + d;
		hash[4] = hash[4] + e;
		hash[5] = hash[5] + f;
		hash[6] = hash[6] + g;
		hash[7] = hash[7] + h;
	}
	
	free(tmp_data);

	data = NULL;
	tmp_data = NULL;

	return hash;
}

hash_t *str2sha256(const char *str) {
	if (!str)
		return NULL;

	size_t length;
	for (length = 0; str[length]; length++);
	
	return bytes2sha256(str, length);
}

char *hash2str(const hash_t *hash) {
	if (!hash) 
		return NULL;

	char *result = (char*)malloc(sizeof(char) * 64);
	if (!result)
		return NULL;
	
	{
		size_t i;
		for (i = 0; i < 64; i++)
			result[i] = hex_sym[ (hash[(i-i%8) / 8] >> (7-i%8) * 4) & 0xf ];
		result[i] = '\0';
	}

	return result;
}

int compareHash(const hash_t *hash1, const hash_t *hash2) {
	if (!hash1 || !hash2)
		return -1;

	for (uint8_t i = 0; i < 8; i++)
		if (hash1[i] != hash2[i])
			return 0;

	return 1;
}

void freeHash(hash_t *hash) {
	if (!hash)
		return;
	
	free(hash);

	return;
}

int printHash(const hash_t *hash) {

	char *hash_str = hash2str(hash);
	
	if (!hash_str)  {
		printf("\033[36mSHA256\033[39m : \033[31mno hash\033[39m");
		return -1;
	}

	printf("\033[36mSHA256\033[39m : %s", hash_str);
	
	free(hash_str);
	hash_str = NULL;

	return 1;
}

uint32_t sha256Mod(const hash_t *hash, uint32_t k) {
	if (!hash)
		return 0;

	uint32_t rest = 0;
	size_t hash_size = sizeof(*hash) * HASH_LENGTH * 8;
	for (size_t i = hash_size; i-->0;) {
		uint32_t num = (rest << 1) | (hash[(hash_size-i-1) / (sizeof(*hash) * 8)] >> i & 1);
		rest = (num >= k) ? num - k : num;
	}
	
	return rest;
}

