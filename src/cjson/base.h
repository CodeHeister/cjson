#ifndef CJSON_BASE_H_
#define CJSON_BASE_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern DataType JSON_type(json_t*);
extern void *JSON_value(json_t*);
extern wchar_t *JSON_key(json_t*);
extern hash_t *JSON_hash(json_t*);
extern size_t JSON_length(json_t*);
extern json_t *node2list(json_t*);
extern json_t *JSON_check_list(json_t*);
extern json_t *JSON_check_json_list(json_t*);
extern void JSON_print(json_t*, PrintFlags);
extern void JSON_info(json_t*, InfoFlags);

#ifdef __cplusplus
}
#endif

#endif
