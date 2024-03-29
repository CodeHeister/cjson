#include <stdio.h>
#include <locale.h>
#include <assert.h>

#include <cjson/cjson.h>
#include <cjson/base.h>

int main() {
	
	// set UTF-8 locale
	setlocale(LC_ALL, "en_US.UTF-8");
	
	// create (dynamic) new json objects
	json_t *new = jsonNew();
	json_t *i = jsonInt(u8"test", 8);
	json_t *u = jsonUint(u8"test", 8);
	json_t *c = jsonChar(u8"test", 'c');
	json_t *s = jsonString(u8"test", u8"тест");

	assert(new);
	assert(i);
	assert(u);
	assert(c);
	assert(s);

	jsonPrint(new, PRINT_PLAIN);
	jsonPrint(i, PRINT_PLAIN);
	jsonPrint(u, PRINT_PLAIN);
	jsonPrint(c, PRINT_PLAIN);
	jsonPrint(s, PRINT_PLAIN);
	printf("\n");

	jsonDelete(new);
	jsonDelete(i);
	jsonDelete(u);
	jsonDelete(c);
	jsonDelete(s);

	return 0;
}
