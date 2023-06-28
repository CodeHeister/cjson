#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <locale.h>

#include <cjson/base.h>
#include <cjson/types.h>
#include <cjson/list.h>

int main() {
	setlocale(LC_ALL, "en_US.UTF-8");
	json_t new;
	jsonNew(&new);
	jsonAdd(jsonInt(u8"привет", 6), &new);
	jsonAdd(jsonUint(u8"hello", 7), &new);
	jsonAdd(jsonNode(u8"holder", NULL), &new);
	jsonAdd(jsonChar(u8"holder", '6'), jsonGet(u8"holder", &new));

	jsonPrint(&new, PRINT_PLAIN);

	int64_t holder = 0;
	uint64_t holder1 = 0;
	jsonGetMultiple(&new,\
			u8"hell", &holder,\
			u8"hello", &holder1,\
			NULL);

	printf("%ld\n", holder);
	printf("%lu\n", holder1);

	jsonFree(&new);

	return 0;
} 
