#include <stdio.h>
#include <locale.h>
#include <assert.h>

#include <cjson/cjson.h>

int main() {
	
	// set UTF-8 locale
	setlocale(LC_ALL, "en_US.UTF-8");
	
	// create (dynamic) new json object
	json_t *new = jsonNew();

	assert(new);

	// add blank node (asian char)
	assert(jsonAdd(jsonNode(u8"ぁ", NULL), new));

	// add int (russian/english/add to node)
	assert(jsonAdd(jsonInt(u8"привет", 6), new));
	assert(jsonAdd(jsonUint(u8"hello", 7), new));
	assert(jsonAdd(jsonChar(u8"holder", '6'), jsonGet(u8"ぁ", new)));

	jsonPrint(new, PRINT_PLAIN);
	printf("\n");

	jsonDelete(new);

	/* ------------------ */

	new = jsonNew();

	assert(new);

	// test node
	json_t *node = jsonNew(NULL);

	assert(node);

	// node
	assert(jsonAdd(jsonInt(u8"test", 6), node));

	// add and merge main with node
	assert(jsonAdd(jsonChar(u8"буква", '5'), new));
	assert(jsonAdd(jsonNode(u8"node", node), new));
	assert(jsonAdd(jsonString(u8"string", u8"hello"), new));

	jsonPrint(new, PRINT_PLAIN);
	printf("\n");

	jsonDelete(new);

	/* ------------------ */

	new = jsonAddMultiple(jsonInt(u8"test", 8),
			jsonInt(u8"test1", 9),
			jsonNode(u8"holder",
				jsonAddMultiple(jsonChar(u8"test", 'c'),
					jsonNew()
					)),
			jsonNew());

	assert(new);

	jsonPrint(new, PRINT_PLAIN);
	printf("\n");

	jsonDelete(new);

	return 0;
} 
