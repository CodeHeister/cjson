#ifndef JSON_LIST_H_
#define JSON_LIST_H_

#include <stdint.h>
#include <stddef.h>

#include "hash/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

struct json_t;

enum data_type {
	NULL_JSON,
	LIST,
	NODE,
	INT,
	UINT,
	CHAR,
	WCHAR,
	DOUBLE,
	STRING,
	WSTRING,
	INT_ARRAY,
	UINT_ARRAY,
	DOUBLE_ARRAY,
	STRING_ARRAY,
	WSTRING_ARRAY,
	UNKNOWN
};

enum info_flags {
	SHOW_TYPE = 0x1,
	SHOW_VALUE = 0x2
};

typedef struct json_t json_t;
typedef enum data_type data_type;
typedef enum info_flags info_flags;
typedef struct value_t value_t;
typedef struct action_t action_t;

extern json_t *json_new();
extern json_t *json_create(wchar_t*, value_t*);
extern json_t *json_get(wchar_t*, json_t*);
extern int json_append(json_t*, json_t*);
extern value_t *json_get_content_of(json_t*);
extern value_t *json_get_content(wchar_t*, json_t*);
extern data_type json_get_type_of(json_t*);
extern data_type json_get_type(wchar_t*, json_t*);
extern size_t json_length(json_t*);
extern json_t *json_chain(wchar_t*, json_t*);
extern value_t *json_chain_content(wchar_t*, json_t*);
extern int json_add(wchar_t*, value_t*, json_t*);
extern void *json_value(value_t*);
extern data_type json_type(value_t*);
extern void json_print(json_t*);
extern value_t *json_int(int);
extern value_t *json_uint(unsigned int);
extern value_t *json_char(char);
extern value_t *json_node();
extern value_t *json_unknown(void*, void (*print)(value_t*), void (*free)(void*), wchar_t *(*convert)(value_t*));
extern int json_remove(wchar_t*, json_t*);

#ifdef JSON_CLASS

typedef struct JSON json_class;

struct JSON {
	json_t *(*new)();
	json_t *(*create)(wchar_t*, value_t*);
	json_t *(*get)(wchar_t*, json_t*);
	int (*append)(json_t*, json_t*);
	value_t *(*getContentOf)(json_t*);
	value_t *(*getContent)(wchar_t*, json_t*);
	data_type (*getTypeOf)(json_t*);
	data_type (*getType)(wchar_t*, json_t*);
	size_t (*length)(json_t*);
	json_t *(*chain)(wchar_t*, json_t*);
	value_t *(*chainContent)(wchar_t*, json_t*);
	int (*add)(wchar_t*, value_t*, json_t*);
	int (*remove)(wchar_t*, json_t*);
	void *(*value)(value_t*);
	data_type (*type)(value_t*);
	void (*print)(json_t*);
	value_t *(*_int_)(int);
	value_t *(*_uint_)(unsigned int);
	value_t *(*_char_)(char);
	value_t *(*_node_)();
	value_t *(*_unknown_)(void*, void (*print)(value_t*), void (*free)(void*), wchar_t *(*convert)(value_t*));
};

json_class JSON = { &json_new, &json_create, &json_get, &json_append, &json_get_content_of, &json_get_content, &json_get_type_of, &json_get_type, &json_length, &json_chain, &json_chain_content,
	&json_add, &json_remove, &json_value, &json_type, &json_print, &json_int, &json_uint, &json_char, &json_node, &json_unknown };

#endif


#ifdef JSON_NO_HASH

typedef wchar_t json_key_t;
int use_hash = 0;

json_key_t *clone_key(const wchar_t *key) {
	uint32_t i = 1;
	while (key[i] != '\0')
		i++;

	wchar_t *clone = (wchar_t*)malloc(sizeof(wchar_t) * i);
	for (uint32_t j = 0; j < i; j++)
		clone[j] = key[j];

	return clone;
}

int cmpstr(const json_key_t *key1, const json_key_t *key2) {
	uint32_t i = 0;
	while (key1[i] != '\0' || key2[i] != '\0') {
		if (key1[i] != key2[i])
			return 0;

		i++;
	}
	if (key1[i] != key2[i])
		return 0;

	return 1;
}

int print_str(const json_key_t *key) {
	if (key == NULL)
		return -1;

	printf("%ls", key);
	return 0;
}

#else

typedef hash_t json_key_t;
int use_hash = 1;

json_key_t *clone_key(const wchar_t *key) {
	return NULL;
}

int cmpstr(const json_key_t *key1, const json_key_t *key2) {
	return 0;
}

int print_str(const json_key_t *key) {
	return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif // JSON_LIST_H_
