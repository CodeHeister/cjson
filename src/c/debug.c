#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>

#include <cjson/debug.h>

void printBytesAsBin(char *bytes, uint64_t size)
{
	if (!bytes)
		return;

	for (uint64_t i = 0; i < size; i++)
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(bytes[i]));
	printf("\n");
	
	return;
}

void *debug_malloc(size_t size, char *fname, uint64_t line)
{
	void *mem = malloc(size);

	if (mem)
		printf("Allocated %zu bytes as %p (%s:%lu)\n", size, mem, fname, line);

	return mem;
}

void debug_free(void *ptr, char* fname, uint64_t line)
{
	if (ptr)
	{
		printf("Freed at %p (%s:%lu)\n", ptr, fname, line);

		free(ptr);
	}

	return;
}
