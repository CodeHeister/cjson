#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <cjson/extra.h>

static int seeded = 0;

size_t rand_pos()
{
	if (!seeded)
	{		
		srand ( time(NULL) );
		seeded = 1;
	}

	return (rand() * HASH_LIMIT) / RAND_MAX;
}

char *clone2str(const char *key)
{
	if (!key)
		return NULL;

	size_t i;
	for (i = 0; key[i] != '\0'; i++);

	char *clone = (char*)malloc(sizeof(char) * (i+1));

	if (!clone)
		return NULL;

	for (size_t j = 0; j <= i; j++)
		clone[j] = key[j];

	return clone;
}

