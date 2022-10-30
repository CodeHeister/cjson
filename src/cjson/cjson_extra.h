#ifndef CJSON_EXTRA_H_
#define CJSON_EXTRA_H_

#include <stddef.h>

#define HASH_LIMIT 4294967295

#ifdef __cplusplus
extern "C" {
#endif

extern size_t rand_pos();
extern wchar_t *clone_key(const wchar_t*);

#ifdef __cplusplus
}
#endif

#endif // CJSON_EXTRA_H_
