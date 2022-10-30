#ifndef CJSON_TYPES_H_
#define CJSON_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

struct json_t;

enum data_type {
	NULL_JSON,
	LIST,
	LINKED_LIST,
	NODE,
	LINKED_LIST_NODE,
	HASH_NODE,
	INT,
	UINT,
	CHAR,
	WCHAR,
	DOUBLE,
	STRING,
	WSTRING,
	UNKNOWN
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
	PRINT_PLAIN = 0x1
};

typedef struct json_t json_t;
typedef enum data_type data_type;
typedef enum info_flags info_flags;
typedef struct value_t value_t;
typedef struct action_t action_t;

#ifdef __cplusplus
}
#endif

#endif // CJSON_TYPES_H_
