#ifndef CJSON_BASE_H_
#define CJSON_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdbool.h>

	#include <cjson/types.h>

	extern json_t *getLast(json_t *);
	extern bool append(json_t *, json_t *);
	extern bool unshift(json_t *, json_t *);

	extern void *infoHash(info_item_t *);
	extern info_type_t infoType(info_item_t *);
	extern void *infoValue(info_item_t *);

	extern bool infoDump(info_t *);
	extern bool infoMove(info_t *, info_t *);
	extern bool infoFree(info_t *);
	extern info_t *infoNew(size_t, info_t *);
	extern info_t *infoResize(int, info_t *);
	extern info_t *infoAdd(char *, info_type_t, void *, info_t *);
	extern info_item_t *infoGet(char *, info_t *);
	extern void *infoGetValue(char *, info_t *);
	extern bool infoRemove(char *, info_t *);
	extern void infoPrintItem(info_item_t *);
	extern void infoPrint(info_t *);

#ifdef __cplusplus
}
#endif

#endif	// CJSON_BASE_H_
