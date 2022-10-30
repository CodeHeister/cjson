#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#include "../src/cjson/cjson.h"
#include "../src/hash/sha256.h"

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru_RU.UTF-8");
	json_t *list = JSON_new();

	JSON_add(L"test", JSON_node(), list);
	JSON_add(L"test", JSON_int(2), JSON_get(L"test", list));

	JSON_info(list, SHOW_LENGTH | SHOW_VALUE | SHOW_TREE | SHOW_HASH | SHOW_KEY);

	hash_t *hash = str2sha256(L"hover");
	print_hash(hash);
	printf("\n%u", hash_rest(hash, UINT_MAX));
	return 0;
}
