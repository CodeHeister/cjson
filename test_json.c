#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stddef.h>

#define JSON_CLASS
#define JSON_NO_HASH

#include "json/json_list.h"


int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru_RU.UTF-8");
	json_t *list = JSON.new();

	JSON.add(L"test", JSON._node_(), list);
	JSON.add(L"test2", JSON._int_(2), JSON.get(L"test", list));

	JSON.print(list);
	return 0;
}
