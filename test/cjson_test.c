#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stddef.h>

#define JSON_CLASS

#include "../src/cjson/cjson.h"

void printKey(json_t *item, wchar_t *key, value_t *content) {
	printf("%ls\n", key);
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru_RU.UTF-8");
	json_t *list = JSON.new();

	JSON.add(L"test", JSON._node_(), list);
	JSON.add(L"test2", JSON._int_(2), JSON.get(L"test", list));
	JSON.add(L"test3", JSON._int_(2), list);
	JSON.add(L"test3", JSON._int_(2), list);

	JSON.print(JSON.get(L"test3", list), 0);
	JSON.forEach(&printKey, list);
	return 0;
}
