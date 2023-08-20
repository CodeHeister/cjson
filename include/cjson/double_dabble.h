#ifndef DOUBLE_DABBLE_H_
#define DOUBLE_DABBLE_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdbool.h>
	#include <inttypes.h>

	#define GETBIT(x, y) (((x) >> (y)) & 1)
	#define CLRBIT(x, y) ((x) &= ~((1) << (y)))
	#define SETBIT(x, y) ((x) |= ((1) << (y)))
	#define GETGRP(x, y) (((x) >> (y*4)) & (0xf))
	#define CLRGRP(x, y) ((x) &= ~((0xf) << (y*4)))
	#define SETGRP(x, y, z) ((x) |= (((y) & (0xf)) << (z*4)))

	typedef struct bigint {
		char *bin;
		uint64_t size;
		uint64_t length;
		bool isNegative;
	} bigint_t;

	extern bigint_t *newBigInt(uint64_t, bigint_t*);
	extern void freeBigInt(bigint_t*);
	extern void deleteBigInt(bigint_t*);
	extern bigint_t *convertToBigInt(char*);
	extern bigint_t *addBigInt(bigint_t*, bigint_t*);
	extern bigint_t *subBigInt(bigint_t*, bigint_t*);

#ifdef __cplusplus
}
#endif

#endif // DOUBLE_DABBLE_H_
