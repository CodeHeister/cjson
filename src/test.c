#include <stdio.h>
#include <stddef.h>

#include <cjson/base.h>
#include <cjson/types.h>
#include <cjson/list.h>

int main() { 
	json_t new;
	jsonNew(&new);
	jsonAdd(jsonInt(L"hell", 6), &new);
	jsonAdd(jsonUint(L"hello", 7), &new);
	jsonAdd(jsonChar(L"hello2", '6'), &new);

	jsonPrint(&new, PRINT_PLAIN);
	jsonPrint(jsonGet(L"hello2", &new), PRINT_PLAIN);

	int64_t holder = 0;
	uint64_t holder1 = 0;
	jsonGetMultiple(&new,\
			L"hell", &holder,\
			L"hello", &holder1,\
			NULL);

	printf("%ld\n", holder);
	printf("%lu\n", holder1);

	jsonFree(&new);

	return 0;
} 
