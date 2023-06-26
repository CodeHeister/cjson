#ifndef CJSON_BASE_H_
#define CJSON_BASE_H_

#include <stdbool.h>

#include <cjson/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern json_t *getLast(json_t*);
extern bool append(json_t*, json_t*);
extern bool unshift(json_t*, json_t*);
extern void *infoGetHash(info_item_t*);
extern info_type_t infoGetType(info_item_t*);
extern void *infoGetValue(info_item_t*);
extern bool infoDump(info_t*);
extern bool infoMove(info_t*, info_t*);
extern bool infoFree(info_t*);
extern info_t *infoNew(size_t, info_t*);
extern info_t *infoResize(int, info_t*);
extern info_t *infoAdd(wchar_t*, info_type_t, void*, info_t*);
extern info_item_t *infoFind(wchar_t*, info_t*);
extern bool infoRemove(wchar_t*, info_t*);

#ifdef __cplusplus
}
#endif

#endif
