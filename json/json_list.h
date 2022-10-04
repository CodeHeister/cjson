#ifndef JSON_LIST_H_
#define JSON_LIST_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct json_t;

enum data_type {
	LIST,
	NODE,
	ARRAY,
	INT,
	UINT,
	DOUBLE,
	STRING,
	UNKNOWN
};

typedef struct json_t json_t;
typedef enum data_type data_type;

extern json_t *new_JSON();
extern json_t *json_create(wchar_t *key, void *value, data_type type);
extern json_t *json_get_by_index(unsigned int index, json_t *list);
extern json_t *json_get_by_key(wchar_t *key, json_t *list);
extern int json_append(json_t *item, json_t *list);
extern void *json_get_value_of(json_t*);
extern void *json_get_value(wchar_t*, json_t*);
extern uint32_t json_length(json_t*);
extern void *json_chain(wchar_t*, json_t*);
extern void *json_chain_value(wchar_t*, json_t*);

#ifdef JSON_CLASS

typedef struct JSON json_class;

struct JSON {
	json_t *(*new)();
	json_t *(*create)(wchar_t*, void*, data_type);
	json_t *(*iget)(unsigned int, json_t*);
	json_t *(*get)(wchar_t*, json_t*);
	int (*append)(json_t*, json_t*);
	void *(*getValueOf)(json_t*);
	void *(*getValue)(wchar_t*, json_t*);
	uint32_t (*length)(json_t*);
	void *(*chain)(wchar_t*, json_t*);
	void *(*chainValue)(wchar_t*, json_t*);
};

json_class JSON = { &new_JSON, &json_create, &json_get_by_index, &json_get_by_key, &json_append, &json_get_value_of, &json_get_value, &json_length, &json_chain, &json_chain_value };

#endif

#ifdef __cplusplus
}
#endif

#endif // JSON_LIST_H_
