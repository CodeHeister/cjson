#ifndef CJSON_LIST_H_
#define CJSON_LIST_H_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include <cjson/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern json_t *jsonCheckJsonList(json_t*);
extern void jsonFree(json_t*);
extern void jsonPrint(json_t*, PrintFlags);
extern bool jsonAdd(json_t*, json_t*);
extern bool jsonMove(json_t*, json_t*);
extern json_t *jsonGet(char*, json_t*);
extern json_t *jsonGetHashNodeByIndex(int, json_t*);
extern void jsonQuickSortHashNodes(int, int, json_t*);
extern void jsonGetMultiple(json_t*, ...);

#ifdef __cplusplus
}
#endif

#endif // CJSON_H_
