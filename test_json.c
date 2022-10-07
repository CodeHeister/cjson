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

	JSON.add(L"test", JSON._node_(), list);
	JSON.add(L"test2", JSON._node_(), JSON.get(L"test", list));
	JSON.add(L"test3", JSON._node_(), JSON.chain(L"test.test2", list));
	JSON.add(L"int", JSON._int_(1), JSON.get(L"test.tester", list));
	JSON.add(L"uint", JSON._uint_(1), JSON.get(L"test", list));
	JSON.add(L"char", JSON._char_('1'), JSON.get(L"test", list));
	JSON.add(L"char", JSON._char_('1'), list);

	JSON.print(list);
	return 0;
}
