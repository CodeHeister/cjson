#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "types.h"
#include "extra.h"

static int seeded = 0;

size_t rand_pos() {
	if (!seeded) {
		srand ( time(NULL) );
		seeded = 1;
	}

	return (rand()*HASH_LIMIT)/RAND_MAX;
}

wchar_t *clone_wstr(const wchar_t *key) {
	if (!key)
		return NULL;

	size_t i = 0;
	while (key[i] != '\0')
		i++;

	wchar_t *clone = (wchar_t*)malloc(sizeof(wchar_t) * (i+1));
	for (size_t j = 0; j <= i; j++)
		clone[j] = key[j];

	return clone;
}

