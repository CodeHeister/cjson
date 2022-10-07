#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "json_list.h"
#include "hash/sha256.h"

#define PRINT_GAP "\t"

static uint32_t print_offset = 0;

struct value_t {
	void *value;
	data_type type;
	void (*print)(value_t *value);
};

struct json_t {
	uint32_t *key;
	value_t *content;
	json_t *next;
};

static json_t *node2list(json_t *node) {
	return (json_t*)(node->content->value);
}

json_t *json_new() {
	json_t *new_list = (json_t*)malloc(sizeof(json_t));
	if (new_list == NULL) {
		return NULL;
	}

	new_list->key = NULL;

	new_list->content = (value_t*)malloc(sizeof(value_t));
	if (new_list->content == NULL) {
		free(new_list);
		return NULL;
	}

	new_list->content->type = LIST;
	new_list->content->value = NULL;
	new_list->content->print = NULL;
	new_list->next = NULL;
	return new_list;
};

json_t *json_create(wchar_t *key, value_t *value) {
	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (item == NULL) {
		return NULL;
	}

	item->key = str2sha256(key);
	if (item->key == NULL) {
		free(item);
		return NULL;
	}

	item->content = (value_t*)malloc(sizeof(value_t));
	if (item->content == NULL) {
		free(item->key);
		free(item);
		return NULL;
	}

	item->next = NULL;
	return item;
}

static json_t *json_get_last(json_t *list) {
	if (list == NULL || list->content->type != LIST) 
		return NULL;

	json_t *item = list;
	while (item->next != NULL)
		item = item->next;
	return item;
}

json_t *json_get(wchar_t *key, json_t *list) {
	if (list == NULL || list->content->type != LIST)
		return NULL;

	uint32_t *hash = str2sha256(key);
	if (hash == NULL)
		return NULL;

	json_t *item = list->next;
	while (item != NULL) {
		if (cmphash(item->key, hash)) {
			free(hash);
			return item;
		}
		
		item = item->next;
	}

	free(hash);
	return NULL;
}

int json_append(json_t *item, json_t *list) {
	if (item == NULL || list == NULL || list->content->type != LIST)
		return -1;

	json_t *iter = list->next;
	while (iter != NULL) {
		if (cmphash(iter->key, item->key)) {
			return -1;
		}
		iter = iter->next;
	}

	if (list->next != NULL) { 
		json_t *last_item = json_get_last(list);
		last_item->next = item;
	}
	else {
		list->next = item;
	}
	
	return 0;
};

value_t *json_get_content_of(json_t* item) {
	if (item == NULL) return NULL;
	return item->content;
}

value_t *json_get_content(wchar_t* key, json_t* list) {
	if (list == NULL || list->content->type != LIST) { 
		return NULL;
	}
	
	json_t *item = json_get(key, list);
	if (item == NULL) return NULL;

	return json_get_content_of(item);
}

size_t json_length(json_t* list) {
	if (list == NULL || list->content->type != LIST) return 0;

	size_t i = 0;
	json_t *item = list->next;
	while (item->next != NULL) {
		item = item->next;
		i++;
	}
	return i;
}

json_t *json_chain(wchar_t *keys, json_t *list) {
	if (list == NULL || list->content->type != LIST)
		return NULL;

	uint32_t cur = 0, i = 0;
	json_t *node = list;

	while (keys[i] != '\0') {
		while (keys[i] != '.') {
			if (keys[i] == '\0') goto break_out;
			i++;
		}
		
		wchar_t *tmp_key = (wchar_t*)malloc( sizeof(wchar_t) * ((i-cur)+1) );
		if (tmp_key == NULL) return NULL;

		uint32_t tmp_cur = 0;
		while (keys[cur] != '.') {
			tmp_key[tmp_cur] = keys[cur];
			tmp_cur++;
			cur++;
		}
		tmp_key[tmp_cur+1] = '\0';
		
		node = json_get(tmp_key, node);
		if (node == NULL || node->content->type != NODE) 
			return NULL;
		
		node = node2list(node);

		free(tmp_key);
		
		if (node == NULL || node->content->type != LIST) 
			return NULL;
		
		cur++;
		i++;
	}
break_out:
	wchar_t *tmp_key = (wchar_t*)malloc( sizeof(wchar_t) * ((i-cur)+2) );
	if (tmp_key == NULL) return NULL;

	uint32_t tmp_cur = 0;
	while (keys[cur] != '\0') {
		tmp_key[tmp_cur] = keys[cur];
		tmp_cur++;
		cur++;
	}
	tmp_key[tmp_cur+1] = '\0';
	
	node = json_get(tmp_key, node);
	free(tmp_key);

	return node;
}


value_t *json_chain_content(wchar_t *keys, json_t *list) {
	return json_get_content_of(json_chain(keys, list));
}

data_type json_get_type_of(json_t *item) {
	if (item == NULL) 
		return NULL_DATA_TYPE;
	return json_type(json_get_content_of(item));
}

data_type json_get_type(wchar_t *key, json_t *list) {
	if (list == NULL || key == NULL || json_get_type_of(list) != LIST)
		return NULL_DATA_TYPE;
	return json_type(json_get_content(key, list));
}

int json_add(wchar_t *key, value_t *content, json_t *list) {
	if (list == NULL || (json_get_type_of(list) != LIST && json_get_type_of(list) != NODE))
		return -1;

	if (list != NULL && list->content->type == NODE)
		list = node2list(list);

	if (content == NULL) {
		printf("JSON.add: no content\n");
		return -1;
	}

	if (list->content->type != LIST) {
		printf("JSON.add: invalid argument pointers\n");
		free(content->value);
		free(content);
		return -1;
	}

	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (item == NULL) {
		printf("JSON.add: item allocation error\n");
		free(content->value);
		free(content);
		return -1;
	}

	item->key = str2sha256(key);
	if (item->key == NULL) {
		printf("JSON.add: hash allocation error\n");
		free(content->value);
		free(content);
		free(item);
		return -1;
	}

	item->content = content;

	if(!json_append(item, list)) { 
		return 0;
	}
	else {
		printf("JSON.add: could not add new item\n");
		free(content->value);
		free(content);
		free(item);
		return -1;
	}
}

static void printInt(value_t *content) {
	printf("%d", *(int*)json_value(content));
}

static void printUint(value_t *content) {
	printf("%u", *(unsigned int*)json_value(content));
}

static void printChar(value_t *content) {
	printf("'%c'", *(char*)json_value(content));
}

static void printList(value_t *content) {
	if (content == NULL || content->value == NULL) return;

	json_t *item = (json_t*)(content->value);
	if (item->content->type != LIST) return;

	item = item->next;
	while (item != NULL) {
		for (uint32_t i = 0; i < print_offset; i++)
			printf(PRINT_GAP);

		if (item->content->type == NODE) {
			print_hash(item->key);
			printf(" : {\n");

			print_offset++;
			if (item->content->print == NULL) 
				printf("NO PRINT\n");
			else
				item->content->print(item->content);
			print_offset--;
			for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
			
			printf("}\n");
		}
		else {
			print_hash(item->key);
			printf(" : ");
			if (item->content->print == NULL) 
				printf("NO PRINT");
			else
				item->content->print(item->content);
			if (item->next != NULL)
				printf(",");
			printf("\n");
		}
		item = item->next;
	}

	return;
}

value_t *json_unknown(void *value, void (*print)(value_t*)) {
	value_t *new_unknown = (value_t*)malloc(sizeof(value_t));
	if (new_unknown == NULL) 
		return NULL;

	new_unknown->type = UNKNOWN;
	new_unknown->value = value;
	new_unknown->print = print;

	return new_unknown;
}

value_t *json_int(int value) {
	value_t *new_int = (value_t*)malloc(sizeof(value_t));
	if (new_int == NULL) 
		return NULL;

	new_int->type = INT;
	new_int->value = malloc(sizeof(value));
	if (new_int == NULL) {
		free(new_int);
		return NULL;
	}
	*(int*)(new_int->value) = value;
	new_int->print = &printInt;

	return new_int;
}

value_t *json_uint(unsigned int value) {
	value_t *new_uint = (value_t*)malloc(sizeof(value_t));
	if (new_uint == NULL) 
		return NULL;

	new_uint->type = INT;
	new_uint->value = malloc(sizeof(value));
	if (new_uint == NULL) {
		free(new_uint);
		return NULL;
	}
	*(unsigned int*)(new_uint->value) = value;
	new_uint->print = &printUint;

	return new_uint;
}

value_t *json_char(char value) {
	value_t *new_char = (value_t*)malloc(sizeof(value_t));
	if (new_char == NULL) 
		return NULL;

	new_char->type = INT;
	new_char->value = malloc(sizeof(value));
	if (new_char == NULL) {
		free(new_char);
		return NULL;
	}
	*(char*)(new_char->value) = value;
	new_char->print = &printChar;

	return new_char;
}

value_t *json_node() {
	value_t *new_node = (value_t*)malloc(sizeof(value_t));
	if (new_node == NULL)
		return NULL;

	new_node->type = NODE;
	new_node->value = json_new();
	if (new_node->value == NULL) {
		free(new_node);
		return NULL;
	}

	new_node->print = &printList;

	return new_node;
}

void *json_value(value_t *value) {
	return value->value;
}

data_type json_type(value_t *value) {
	return value->type;
}

void json_print(json_t *item) {
	if (item == NULL) 
		return;

	printf("\n");
	if (item->content->type == LIST) {
		if (print_hash(item->key) < 0) {
			printf("{\n");
		}
		else {
			printf(" : {\n");
		}
		print_offset++;
		item = item->next;
		while (item != NULL) {
			for (uint32_t i = 0; i < print_offset; i++)
				printf(PRINT_GAP);

			if (item->content->type == NODE) {
				print_hash(item->key);
				printf(" : {\n");

				print_offset++;
				if (item->content->print == NULL) 
					printf("NO PRINT\n");
				else
					item->content->print(item->content);
				print_offset--;
				for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
				
				printf("}\n");
			}
			else {
				print_hash(item->key);
				printf(" : ");
				if (item->content->print == NULL) 
					printf("NO PRINT");
				else
					item->content->print(item->content);
				if (item->next != NULL)
					printf(",");
				printf("\n");
			}
			item = item->next;
		}
		print_offset--;
		printf("}\n");
	}
	else {
		if (item->content->print == NULL) 
			printf("NO PRINT\n");
		else
			item->content->print(item->content);
		printf("\n");
	}

	return;
}

static int json_free(json_t *item) {
	if (item == NULL || json_get_type_of(item) == NODE || json_get_type_of(item) == LIST)
		return -1;

	if (item->key != NULL) 
		free(item->key);
	if (item->content != NULL && item->content->value != NULL) {
		free(item->content->value);
		free(item->content);
	}
	free(item);

	return 1;
}
static int json_remove_list(json_t*);
static int json_remove_node(json_t*);

static int json_remove_node(json_t *node) {
	if (node == NULL || json_get_type_of(node) != LIST) {
		printf("JSON.removeList: not a node");
		return -1;
	}

	json_remove_list(node2list(node));
	json_free(node);

	return 0;
}

static int json_remove_list(json_t *list) {
	if (list == NULL || json_get_type_of(list) != LIST) {
		printf("JSON.removeList: not a list");
		return -1;
	}

	json_t *item = list->next;
	json_t *save_item = item->next;
	while (item != NULL) {
		if (json_get_type_of(item) == NODE) {
			json_remove_node(item);
		}
		else {
			json_free(item);
		}

		item = save_item;
		save_item = item->next;
	}
	json_free(list);

	return 1;
}

int json_remove(wchar_t *key, json_t *list) {
	if (list == NULL || key == NULL || (json_get_type_of(list) != LIST && json_get_type_of(list) != NODE)) {
		printf("JSON.remove: invalid arguments");
		return -1;
	}

	if (json_get_type_of(list) == NODE)
		list = node2list(list);

	json_t *save_item = list;
	json_t *item = list->next;
	uint32_t *hash = str2sha256(key);
	while (item != NULL) {
		if (cmphash(item->key, hash)) {
			break;
		}

		save_item = item;
		item = item->next;
	}
	free(hash);

	if (item == NULL) {
		printf("JSON.remove: no such element");
		return -1;
	}

	data_type type = json_get_type_of(item);
	if (type == NODE) {
		save_item->next = item->next;
		json_remove_node(item);
	}
	else if (type == LIST) {
		save_item->next = item->next;
		json_remove_list(item);
	}
	else if (type == UNKNOWN) {

	}
	else {
		save_item->next = item->next;
		json_free(item);
	}

	return 0;
}

