#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stddef.h>

#define JSON_CLASS

#include "json/json_list.h"
#include "json/hash/sha256.h"

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru_RU.UTF-8");
	json_t *list = JSON.new();

	JSON.append(JSON.create(L"user", JSON.new(), NODE), list);
	JSON.append(JSON.create(L"id", 1, INT), JSON.getValue(L"user", list));
	JSON.append(JSON.create(L"name", "Sergei", STRING), JSON.getValue(L"user", list));
	
	printf("%d. %s\n", (int)JSON.chainValue(L"user.id", list), (char*)JSON.chainValue(L"user.name", list));
	return 0;
}
