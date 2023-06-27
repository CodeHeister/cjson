#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <cjson/types.h>
#include <cjson/extra.h>

static int seeded = 0;

size_t rand_pos() {
	if (!seeded) {
		srand ( time(NULL) );
		seeded = 1;
	}

	return (rand()*HASH_LIMIT)/RAND_MAX;
}

wchar_t *cloneWstr(const wchar_t *key) {
	if (!key)
		return NULL;

	size_t i;
	for (i = 0; key[i] != '\0'; i++);

	wchar_t *clone = (wchar_t*)malloc(sizeof(wchar_t) * (i+1));

	if (!clone)
		return NULL;

	for (size_t j = 0; j <= i; j++)
		clone[j] = key[j];

	return clone;
}

