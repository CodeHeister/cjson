#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#include <sys/time.h>

#include "../src/cjson/cjson.h"
#include "../src/cjson/config.h"
#include "../src/hash/sha256.h"

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru_RU.UTF-8");
	json_t *list = JSON_new();
	/*printf("%lu\n", list->item.json_list->hash_length);
	for (json_t *hash_node = list->next; hash_node != NULL; hash_node=hash_node->next)
		printf("%u ", hash_node->item.hash_node->pos);
	printf("\n");
	wchar_t *test = (wchar_t*)malloc(sizeof(wchar_t)* 4);
	test[0] = L'h';
	test[1] = L'i';
	test[2] = L'1';
	test[3] = L'\0';
	for (uint32_t i = 0; i < 1000000; i++) {
		struct timeval begin, end;
		gettimeofday(&begin, 0);

		if (JSON_add(JSON_int(test, 4), list) < 0)
			printf("Error");
		gettimeofday(&end, 0);

		long seconds = end.tv_sec - begin.tv_sec;
		long microseconds = end.tv_usec - begin.tv_usec;
		double elapsed = seconds + microseconds*1e-6;
		printf("Time measured: %.3f seconds.\n", elapsed);
	}*/
	JSON_add(JSON_int(L"hi", 3), list);
	JSON_add(JSON_int(L"hi", 5), list);
	JSON_add(JSON_int(L"hello345", 3), list);
	JSON_add(JSON_char(L"geek5", 'h'), list);
	JSON_info(list, SHOW_LENGTH | SHOW_VALUE | SHOW_HASH);

	return 0;
}
