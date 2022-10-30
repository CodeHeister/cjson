#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "cjson_types.h"
#include "cjson_extra.h"

static int seeded = 0;

size_t rand_pos() {
	if (!seeded) {
		srand ( time(NULL) );
		seeded = 1;
	}

	return (rand()*HASH_LIMIT)/RAND_MAX;
}

wchar_t *clone_key(const wchar_t *key) {
	if (!key)
		return NULL;

	uint32_t i = 1;
	while (key[i] != '\0')
		i++;

	wchar_t *clone = (wchar_t*)malloc(sizeof(wchar_t) * i);
	for (uint32_t j = 0; j < i; j++)
		clone[j] = key[j];

	return clone;
}

