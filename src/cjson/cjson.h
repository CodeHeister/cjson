#ifndef CJSON_H_
#define CJSON_H_

#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "cjson_types.h"
#include "../hash/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

extern json_t *JSON_new();
extern json_t *JSON_create(wchar_t*, value_t*);
extern json_t *JSON_get(wchar_t*, json_t*);
extern int JSON_append(json_t*, json_t*);
extern value_t *JSON_get_content_of(json_t*);
extern value_t *JSON_get_content(wchar_t*, json_t*);
extern data_type JSON_get_type_of(json_t*);
extern data_type JSON_get_type(wchar_t*, json_t*);
extern size_t JSON_length(json_t*);
extern json_t *JSON_chain(wchar_t*, json_t*);
extern value_t *JSON_chain_content(wchar_t*, json_t*);
extern int JSON_add(wchar_t*, value_t*, json_t*);
extern void *JSON_value(value_t*);
extern data_type JSON_type(value_t*);
extern void JSON_print(json_t*, uint32_t);
extern value_t *JSON_int(int);
extern value_t *JSON_uint(unsigned int);
extern value_t *JSON_char(char);
extern value_t *JSON_node();
extern value_t *JSON_unknown(void*, void (*print)(value_t*, uint32_t flags), void (*free)(void*), wchar_t *(*convert)(value_t*));
extern int JSON_remove(const wchar_t*, json_t*);
extern void JSON_free_list(void*);
extern int JSON_for_each(void (*iter)(json_t*, wchar_t*, value_t*), json_t*);
extern void JSON_info(json_t*, info_flags);

#ifdef __cplusplus
}
#endif

#endif // CJSON_H_
