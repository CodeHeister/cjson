#ifndef CJSON_TYPES_H_
#define CJSON_TYPES_H_

#include <stdint.h>
#include <stddef.h>

#include <chash/sha256.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum type {
	INT,
	UINT,
	CHAR,
	WCHAR,
	DOUBLE,
	BLANK,
	STRING,
	WSTRING,
	HASH,
	PTR,
} info_type_t;

typedef enum json_type {
	ITEM,
	LIST,
	NODE,
	HASH_NODE,
	ARRAY,
	UNKNOWN
} type_t;

typedef enum info_flags {
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

typedef enum print_flags {
	PRINT_PLAIN,
} PrintFlags;

typedef struct info_item {
	hash_t *hash;
	info_type_t type;
	union {
		void *value;
		double f64;
		int64_t i64;
		uint64_t u64;
		char c;
		wchar_t wc;
	};
} info_item_t;

typedef struct info {
	info_item_t *array;
	size_t length;
} info_t;

typedef struct json json_t;
typedef struct json_ftable json_ftable_t;

typedef struct json {
	info_t info;
	type_t type;
	union {
		void *value;
		double f64;
		int64_t i64;
		uint64_t u64;
		char c;
		wchar_t wc;
	};
	json_ftable_t *vtable;
	json_t *next;
} json_t;

typedef struct json_ftable {
	void (*print)(json_t*, PrintFlags);	// print function
	void (*free)(json_t*);
	wchar_t *(*convert)(json_t*);
} json_ftable_t;

extern type_t jsonGetType(json_t*);
extern info_t *jsonGetInfo(json_t*);
extern void *jsonGetValue(json_t*);
extern void jsonPrint(json_t*, PrintFlags);
extern json_t *jsonNew(json_t*);
extern json_t *jsonHashNode();
extern json_t *jsonUnknown(void*, json_ftable_t*);
extern json_t *jsonInt(const wchar_t*, int64_t);
extern json_t *jsonUint(const wchar_t*, uint64_t);
extern json_t *jsonChar(const wchar_t*, char);

#ifdef __cplusplus
}
#endif

#endif // CJSON_H_
