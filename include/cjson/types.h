#ifndef CJSON_TYPES_H_
#define CJSON_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdint.h>
	#include <stdbool.h>
	#include <uchar.h>

	#include <cjson/sha256.h>

	typedef enum type
	{
		INT,
		UINT,
		CHAR,
		WCHAR,
		DOUBLE,
		STATIC,
		BLANK,
		STRING,
		WSTRING,
		HASH,
		PTR,
	} info_type_t;

	typedef enum json_type
	{
		ITEM,
		LIST,
		NODE,
		HASH_NODE,
		ARRAY,
		UNKNOWN
	} type_t;

	typedef enum info_flags
	{
		SHOW_TYPE = 0x1,
		SHOW_VALUE = 0x2,
		SHOW_NODES = 0x4,
		SHOW_MAP = 0x8,
		SHOW_LENGTH = 0x10,
		SHOW_TREE = 0x20,
		SHOW_HASH = 0x40,
		SHOW_KEY = 0x80,
		SHOW_ALL = 0xff,
	} InfoFlags;

	typedef enum print_flags
	{
		PRINT_PLAIN,
		PRINT_COLORED,
		PRINT_INLINE,
	} PrintFlags;

	typedef struct info_item
	{
		hash_t *hash;
		info_type_t type;
		void *value;
	} info_item_t;

	typedef struct info
	{
		info_item_t *array;
		uint64_t length;
	} info_t;

	typedef struct json json_t;
	typedef struct json_ftable json_ftable_t;

	typedef struct json
	{
		info_t info;
		type_t type;
		
		void *value;
		
		json_ftable_t *vtable;
		
		json_t *next;
	} json_t;

	typedef struct json_ftable
	{
		void (*print)(json_t *, PrintFlags);
		void (*free)(json_t *);
		char *(*convert)(json_t *);
		json_t *(*clone)(json_t *);
		json_t *(*get)(void *, json_t *);
		bool (*add)(json_t *, json_t *);
	} json_ftable_t;

	extern type_t jsonType(json_t *);
	extern info_t *jsonInfo(json_t *);
	extern void *jsonValue(json_t *);

	extern json_t *jsonUnknown(void *, json_ftable_t *, json_t *);

	extern json_t *jsonNew(json_t *);
	extern json_t *jsonNode(const char *, json_t *);
	extern json_t *jsonHashNode();

	extern json_t *jsonInt(const char *, int64_t);
	extern json_t *jsonUint(const char *, uint64_t);
	extern json_t *jsonChar(const char *, char);
	extern json_t *jsonString(const char *, char *);

#ifdef __cplusplus
}
#endif

#endif // CJSON_TYPES_H_
