#include <stdio.h>
#include <locale.h>

#include <cjson/cjson.h>

int main() {
	
	// set UTF-8 locale
	setlocale(LC_ALL, "en_US.UTF-8");
	
	// create (dynamic) new json object
	json_t *new = jsonNew(NULL);

	// add blank node (asian char)
	jsonAdd(jsonNode(u8"ぁ", NULL), new);

	// add int (russian/english/add to node)
	jsonAdd(jsonInt(u8"привет", 6), new);
	jsonAdd(jsonUint(u8"hello", 7), new);
	jsonAdd(jsonChar(u8"holder", '6'), jsonGet(u8"ぁ", new));

	jsonPrint(new, PRINT_PLAIN);

	jsonDelete(new);

	printf("\n");

	new = jsonNew(NULL);

	// test node
	json_t *node = jsonNew(NULL);

	// node
	jsonAdd(jsonInt(u8"test", 6), node);

	// add and merge main with node
	jsonAdd(jsonChar(u8"буква", '5'), new);
	jsonAdd(jsonNode(u8"node", node), new);

	jsonPrint(new, PRINT_PLAIN);

	jsonDelete(new);

	return 0;
} 
