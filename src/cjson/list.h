#ifndef CJSON_LIST_H_
#define CJSON_LIST_H_

#include <stdint.h>
#include <stddef.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern json_t *JSON_get_last(json_t*);
extern int JSON_append(json_t*, json_t*);
extern int JSON_unshift(json_t*, json_t*);
extern void JSON_quick_sort_hash_nodes(json_t*, int, int);
extern int JSON_add(json_t*, json_t*);
extern int JSON_remove(const wchar_t*, json_t*);
extern json_t *JSON_get(wchar_t*, json_t*);
extern void JSON_free(json_t*);

#ifdef __cplusplus
}
#endif

#endif // CJSON_H_
