#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <inttypes.h>
#include <stdio.h>

#include "sha256.h"

#define Ch(x, y, z) ((x & (y ^ z)) ^ z)
#define Maj(x, y, z)    ((x & (y | z)) | (y & z))
#define SHR(x, n)   (x >> n)
#define ROTR(x, n)  ((x >> n) | (x << (32 - n)))
#define S0(x)       (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)       (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)       (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
#define s1(x)       (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))

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

uint32_t *str2sha256(const wchar_t *key) {
	// create data pointer
	uint32_t *data = NULL;

	// create var for 512 bit block amount
	uint32_t len_blk = 0;

	// create byte size var (including extra byte and size bytes) 
	uint32_t size = 9;
	
	// hash storage for calculations
	uint32_t *hash = (uint32_t*)malloc(sizeof(uint32_t) * 8);
	hash[0] = 0x6A09E667;
    hash[1] = 0xBB67AE85;
    hash[2] = 0x3C6EF372;
    hash[3] = 0xA54FF53A;
    hash[4] = 0x510E527F;
    hash[5] = 0x9B05688C;
    hash[6] = 0x1F83D9AB;
    hash[7] = 0x5BE0CD19;

	{	
		// iterator
		uint32_t i = 0;
		while (key[i] != '\0') {
			
			// size calculations excluding zero bytes
			size += (uint32_t)ceil(ceil(log((double)key[i])/log(2))/8);
			i++;
		}
		
		// 512 bit blocks amount
		len_blk += (int)ceil(size*8/512.0);

		// 8 bit data array
		uint8_t *tmp_data = (uint8_t*)malloc(len_blk * 512);

		// check if memory was allocated
		if (tmp_data == NULL) {
			return NULL;
		}

		// wipe garbage
		for (int j = 0; j < len_blk * 64; j++)
			tmp_data[j] = 0;

		// drop iterators
		i = 0;
		uint32_t j = 0;

		// fill 8 bit array
		while (key[i] != '\0') {
			// get current char size excluding zero bytes
			uint32_t tmp_char_size = (int)ceil(ceil(log((double)key[i])/log(2))/8);

			// iterate through char as 8 bit blocks
			for (int g = tmp_char_size; g > 0; g--) {
				// add 8 bit data in backward direction to 32 bit 
				tmp_data[j/4*4+3-j%4] = (uint8_t)(key[i] >> (g-1) * 8);
				j++;
			}
			i++;
		}
		// add extra byte
		tmp_data[j/4*4+3-j%4] = 0b10000000;

		{
			// get size from byte size
			uint32_t tmp_limit = (int)ceil(ceil(log((double)(size-9) * 8)/log(2)) / 8);

			// get last index
			int tmp_size = len_blk*64-1;

			// get byte size for byte size
			uint32_t tmp_bit_size = (size-9) * 8;

			for (int g = 0; g < tmp_limit; g++)
				// iterate to add byte size to the end of data in backward direction to 32 bit
				tmp_data[tmp_size-(3-g%4)] = (uint8_t)(tmp_bit_size >> g * 8);
		}

		// move 8 bit data to 32 bit
		data = (uint32_t*)tmp_data;
	}

	uint32_t *tmp_data = (uint32_t*)malloc(sizeof(uint32_t) * 64);
	if (tmp_data == NULL) {
		return NULL;
	}

	for (uint32_t i = 0; i < len_blk; i++) {

		uint32_t a = hash[0];
		uint32_t b = hash[1];
		uint32_t c = hash[2];
		uint32_t d = hash[3];
		uint32_t e = hash[4];
		uint32_t f = hash[5];
		uint32_t g = hash[6];
		uint32_t h = hash[7];

		for (uint8_t j = 0; j < 16; j++) 
			tmp_data[j] = data[i*16+j];
		
		for (uint8_t j = 0; j < 48; j++)
			tmp_data[j+16] = tmp_data[j] + s0(tmp_data[j+1]) + tmp_data[j+9] + s1(tmp_data[j+14]);

		for (uint8_t j = 0; j < 64; j++) {
			uint32_t tmp1 = h + S1(e) +	Ch(e, f, g) + tmp_data[j] + K[j];
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
	free(data);

	return hash;
}

unsigned char *hash2str(const uint32_t *hash) {
	unsigned char *result = (unsigned char*)malloc(sizeof(unsigned char) * 65);
	result[64] = '\0';
	if (result == NULL) {
		return NULL;
	}

	for (int i = 0; i < 64; i++) 
		result[i] = hex_sym[(hash[i/8] >> (7-i%8)*4) & 15];

	return result;
}

int cmphash(const uint32_t *hash1, const uint32_t *hash2) {
	for (uint8_t i = 0; i < 8; i++)
		if (hash1[i] != hash2[i]) return 0;
	return 1;
}

int print_hash(const uint32_t *hash) {
	unsigned char *hash_str = hash2str(hash);
	if (hash_str == NULL) 
		return -1;
	printf("sha256:%s", hash_str);
	free(hash_str);
	return 0;
}
