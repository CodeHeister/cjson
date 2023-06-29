#include <stdio.h>
#include <locale.h>

#include <cjson/cjson.h>

int main() {
	
	// set UTF-8 locale
	setlocale(LC_ALL, "en_US.UTF-8");
	
	// create (dynamic) new json objects
	json_t *new = jsonNew(NULL);
	json_t *i = jsonInt(u8"test", 8);
	json_t *u = jsonUint(u8"test", 8);
	json_t *c = jsonChar(u8"test", 'c');
	json_t *s = jsonString(u8"test", u8"тест");

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
