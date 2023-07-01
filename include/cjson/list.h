#ifndef CJSON_LIST_H_
#define CJSON_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdarg.h>
	#include <stddef.h>
	#include <stdint.h>
	#include <stdbool.h>

	#include <cjson/types.h>

	extern json_t *jsonCheckIfList(json_t *);

	extern void jsonFree(json_t *);
	extern void jsonDelete(json_t *);
	extern bool jsonMove(json_t *, json_t *);
	extern void jsonPrint(json_t *, PrintFlags);

	extern bool jsonAdd(json_t *, json_t *);
	extern json_t *jsonAddMultiple(json_t *, ...);

	extern json_t *jsonGet(const char *, json_t *);
	extern void *jsonGetValue(char *, json_t *);
	extern bool jsonGetMultiple(json_t *, ...);
	extern bool jsonGetMultipleValues(json_t *, ...);

	extern json_t *jsonGetHashNodeByIndex(int, json_t *);
	extern void jsonQuickSortHashNodes(int, int, json_t *);

#ifdef __cplusplus
}
#endif

#endif // CJSON_LIST_H_
