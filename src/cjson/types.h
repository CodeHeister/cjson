#ifndef CJSON_TYPES_H_
#define CJSON_TYPES_H_

#include <stdint.h>
#include <stddef.h>

#include "../hash/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

enum data_type {
	NULL_JSON,
	LIST,
	LINKED_LIST,
	LL_INT,
	LL_UINT,
	LL_CHAR,
	LL_WCHAR,
	LL_DOUBLE,
	LL_STRING,
	LL_WSTRING,
	HASH_NODE,
	NODE,
	LL_NODE,
	INT,
	UINT,
	CHAR,
	WCHAR,
	DOUBLE,
	STRING,
	WSTRING,
	UNKNOWN,
};

enum info_flags {
	SHOW_TYPE = 0x1,
	SHOW_VALUE = 0x2,
	SHOW_NODES = 0x4,
	SHOW_MAP = 0x8,
	SHOW_LENGTH = 0x10,
	SHOW_TREE = 0x20,
	SHOW_HASH = 0x40,
	SHOW_KEY = 0x80,
};

enum print_flags {
	PRINT_PLAIN = 0x1,
};

/* flags and types types */

typedef enum data_type DataType;
typedef enum info_flags InfoFlags;
typedef enum print_flags PrintFlags;

/* pre defined types */ 

typedef struct json json_t;
typedef struct json_ftable json_ftable_t;
typedef struct json_item json_item_t;
typedef struct json_list json_list_t;
typedef struct json_list_item json_list_item_t;
typedef struct linked_list linked_list_t;
typedef struct linked_list_item linked_list_item_t;
typedef struct node node_t;
typedef struct hash_node hash_node_t;

struct json_ftable {
	void (*print)(json_t*, PrintFlags);	// print function
	void (*free)(json_t*);
	wchar_t *(*convert)(json_t*);
};

/* main type */

struct json {
	DataType type;
	union {
		json_item_t *json_item;
		json_list_t *json_list;
		json_list_item_t *json_list_item;
		linked_list_t *linked_list;
		linked_list_item_t *linked_list_item;
		node_t *node;
		hash_node_t *hash_node;
		void *unknown;
	} item;
	json_ftable_t *vtable;
	json_t *next;
};

/* json list type */

struct json_item {
	wchar_t *key;
	hash_t *hash;
	void *value;
};

/* json list type */

struct json_list {
	size_t length;
	size_t hash_length;
};

/* linked list type */

struct linked_list {
	size_t length;
};

/* linked list item type */

struct linked_list_item {
	void *value;
};

/* hash node / node type */

struct node {
	hash_t *hash;	// json hash
	wchar_t *key;	// json key
	json_t *item;
};

struct hash_node {
	uint32_t pos;
	json_t *item;
};

extern json_t *JSON_node(wchar_t*);
extern json_t *JSON_hash_node();
extern json_t *JSON_new();
extern json_t *JSON_unknown(wchar_t*, void*, json_ftable_t*);
extern json_t *JSON_int(wchar_t*, int);
extern json_t *JSON_uint(wchar_t*, unsigned int);
extern json_t *JSON_char(wchar_t*, char);

#ifdef __cplusplus
}
#endif

#endif // CJSON_H_
