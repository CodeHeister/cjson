#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "json_list.h"

/* output configuraions */

#define PRINT_GAP "\t"

static uint8_t print_offset = 0;

extern int use_hash;

/* content type for item */

struct action_t {
	void (*print)(value_t*);
	void (*free)(void*);
	wchar_t *(*convert)(value_t*);
};

struct value_t {
	void *value;
	data_type type;
	action_t actions;
};

/* base item type */

struct json_t {
	json_key_t *key;
	value_t *content;
	json_t *next;
};

/* node in list translation */

static json_t *node2list(json_t *node) {
	if (node == NULL || json_get_type_of(node) != NODE)
		return NULL;

	return (json_t*)(node->content->value);
}

/* create new list */

json_t *json_new() {
	json_t *new_list = (json_t*)malloc(sizeof(json_t));
	if (new_list == NULL)
		return NULL;

	new_list->content = (value_t*)malloc(sizeof(value_t));
	if (new_list->content == NULL) {
		free(new_list);
		return NULL;
	}

	new_list->key = NULL;
	new_list->content->type = LIST;
	new_list->content->value = NULL;
	new_list->content->actions.print = NULL;
	new_list->content->actions.free = NULL;
	new_list->content->actions.convert = NULL;
	new_list->next = NULL;
	return new_list;
};

/* get value from content */

void *json_value(value_t *content) {
	if (content == NULL) 
		return NULL;

	return content->value;
}

/* get type from content */

data_type json_type(value_t *content) {
	if (content == NULL) 
		return NULL_JSON;

	return content->type;
}

/* get content from item */

value_t *json_get_content_of(json_t* item) {
	if (item == NULL)
		return NULL;
	
	return item->content;
}

/* get type from item */

data_type json_get_type_of(json_t *item) {
	if (item == NULL) 
		return NULL_JSON;

	return json_type(json_get_content_of(item));
}

/* get value from item */

void *json_get_value_of(json_t *item) {
	if (item == NULL) 
		return NULL;

	return json_value(json_get_content_of(item));
}

/* check if list is valid (also node -> list) */

static json_t *json_check_list(json_t *list) {
	if (list == NULL || (json_get_type_of(list) != LIST && json_get_type_of(list) != NODE))
		return NULL;
		
	if (json_get_type_of(list) == NODE)
		list = node2list(list);

	return list;
}

/* get content fomr key and list */

value_t *json_get_content(wchar_t* key, json_t* list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return NULL;

	json_t *item = json_get(key, list);
	return json_get_content_of(item);
}

/* get type from key and list */

data_type json_get_type(wchar_t *key, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return NULL_JSON;

	return json_type(json_get_content(key, list));
}

/* get value from key and list */

void *json_get_value(wchar_t *key, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return NULL;

	return json_value(json_get_content(key, list));
}

/* check if item with such name exists in list */

int json_key_exists(json_key_t *key, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return -1;

	json_t *item = list->next;
	while (item != NULL) {
		
		int cmp;
		if (use_hash)
			cmp = cmphash((hash_t*)item->key, (hash_t*)key);
		else
			cmp = cmpstr(item->key, key);

		if (cmp)
			return 1;

		item = item->next;
	}

	return 0;
}

/* lsit length */

size_t json_length(json_t* list) {
	list = json_check_list(list);
	if (list == NULL)
		return 0;
	
	size_t length = 0;
	json_t *item = list->next;
	while (item != NULL) {
		length++;
		item = item->next;
	}

	return length;
}

/* item assembling */

json_t *json_create(wchar_t *key, value_t *content) {
	if (key == NULL || content == NULL)
		return NULL;

	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (item == NULL)
		return NULL;

	if (use_hash)
		item->key = (json_key_t*)str2sha256(key);
	else
		item->key = clone_key(key);

	item->content = (value_t*)malloc(sizeof(value_t));
	if (item->key == NULL || item->content == NULL) {
		free(item->content);
		free(item->key);
		free(item);
		return NULL;
	}

	item->content = content;
	item->next = NULL;
	return item;
}

/* free any item */

int json_free(json_t *item) {
	if (item == NULL)
		return -1;

	if (item->key != NULL) 
		free(item->key);

	value_t *content = json_get_content_of(item);
	void *value = json_value(content);
	if (content != NULL && value != NULL) {
		
		if (content->actions.free == NULL)
			free(value);
		else
			content->actions.free(value);

		free(item->content);
	}
	free(item);

	return 1;
}

/* free list item */

static void json_free_list(void *value) {
	if (value == NULL)
		return;

	json_t *list = (json_t*)value;

	json_t *item = list->next;
	while (item != NULL) {
		printf("%d\n", json_free(item));

		item = item->next;
	}
	free(list);

	return;
}

/* get last item in list */

json_t *json_get_last(json_t *list) {
	list = json_check_list(list);
	if (list == NULL) 
		return NULL;

	json_t *item = list;
	while (item->next != NULL)
		item = item->next;

	return item;
}

/* get item in list from key */

json_t *json_get(wchar_t *key, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return NULL;

	json_key_t *hash;
	if (use_hash)
		hash = (json_key_t*)str2sha256(key);
	else
		hash = clone_key(key);

	if (hash == NULL)
		return NULL;

	json_t *item = list->next;
	while (item != NULL) {
		
		int cmp;
		if (use_hash)
			cmp = cmphash((hash_t*)item->key, (hash_t*)hash);
		else
			cmp = cmpstr(item->key, hash);

		if (cmp) {
			free(hash);
			return item;
		}

		item = item->next;
	}

	free(hash);
	return NULL;
}

/* append new item to list */

int json_append(json_t *item, json_t *list) {
	list = json_check_list(list);
	if (list == NULL)
		return -1;

	if (json_key_exists(item->key, list))
		return 0;

	json_t *last_item = json_get_last(list);
	if (last_item == NULL)
		return -1;
	
	last_item->next = item;
	return 1;
};

/* add new item to list */

int json_add(wchar_t *key, value_t *content, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL || content == NULL) {
		free(content->value);
		free(content);
		return -1;
	}

	json_t *item = json_create(key, content);

	int append_status = json_append(item, list);
	if(append_status < 1) { 
		json_free(item);
		return append_status;
	}

	return 1;
}

/* remove an item from list */

int json_remove(wchar_t *key, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || key == NULL)
		return -1;

	json_t *save_item = list;
	json_t *item = list->next;
	json_key_t *hash;
	if (use_hash)
		hash = (json_key_t*)str2sha256(key);
	else
		hash = clone_key(key);

	while (item != NULL) {

		int cmp;
		if (use_hash)
			cmp = cmphash((hash_t*)item->key, (hash_t*)hash);
		else
			cmp = cmpstr(item->key, hash);
		
		if (cmp)
			break;

		save_item = item;
		item = item->next;
	}
	free(hash);

	if (item == NULL)
		return 0;

	save_item->next = item->next;
	return json_free(item);
}

/* advanced search item content */

value_t *json_chain_content(wchar_t *keys, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || keys == NULL)
		return NULL;

	return json_get_content_of(json_chain(keys, list));
}

data_type json_chain_type(wchar_t *keys, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || keys == NULL)
		return NULL_JSON;

	return json_get_type_of(json_chain(keys, list));
}

void *json_chain_value(wchar_t *keys, json_t *list) {
	list = json_check_list(list);
	if (list == NULL || keys == NULL)
		return NULL;

	return json_get_value_of(json_chain(keys, list));
}

/* print int */

static void printInt(value_t *content) {
	printf("%d", *(int*)json_value(content));
}

/* print unsigned int */

static void printUint(value_t *content) {
	printf("%u", *(unsigned int*)json_value(content));
}

/* print char */

static void printChar(value_t *content) {
	printf("'%c'", *(char*)json_value(content));
}

/* int type */

value_t *json_int(int value) {
	value_t *new_int = (value_t*)malloc(sizeof(value_t));
	if (new_int == NULL) 
		return NULL;

	new_int->type = INT;
	new_int->value = malloc(sizeof(value));
	if (new_int->value == NULL) {
		free(new_int);
		return NULL;
	}
	*(int*)(new_int->value) = value;
	new_int->actions.print = &printInt;
	new_int->actions.free = NULL;

	return new_int;
}

/* unsigned int type */

value_t *json_uint(unsigned int value) {
	value_t *new_uint = (value_t*)malloc(sizeof(value_t));
	if (new_uint == NULL) 
		return NULL;

	new_uint->type = INT;
	new_uint->value = malloc(sizeof(value));
	if (new_uint->value == NULL) {
		free(new_uint);
		return NULL;
	}
	*(unsigned int*)(new_uint->value) = value;
	new_uint->actions.print = &printUint;
	new_uint->actions.free = NULL;

	return new_uint;
}

/* char type */

value_t *json_char(char value) {
	value_t *new_char = (value_t*)malloc(sizeof(value_t));
	if (new_char == NULL) 
		return NULL;

	new_char->type = INT;
	new_char->value = malloc(sizeof(value));
	if (new_char->value == NULL) {
		free(new_char);
		return NULL;
	}
	*(char*)(new_char->value) = value;
	new_char->actions.print = &printChar;
	new_char->actions.free = NULL;

	return new_char;
}

static void printNode(value_t *content);

/* node (list in list) type */

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

	new_node->actions.print = &printNode;
	new_node->actions.free = &json_free_list;

	return new_node;
}

/* unknown type */

value_t *json_unknown(void *value, void (*print)(value_t*), void (*free)(void*), wchar_t *(*convert)(value_t*)) {
	value_t *new_unknown = (value_t*)malloc(sizeof(value_t));
	if (new_unknown == NULL) 
		return NULL;

	new_unknown->type = UNKNOWN;
	new_unknown->value = value;
	new_unknown->actions.print = print;
	new_unknown->actions.free = free;
	new_unknown->actions.convert = convert;

	return new_unknown;
}

/* ----------------- */

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


static void printNode(value_t *content) {
	if (content == NULL || content->value == NULL) return;

	json_t *item = (json_t*)(content->value);
	if (item->content->type != LIST) return;

	item = item->next;
	while (item != NULL) {
		for (uint32_t i = 0; i < print_offset; i++)
			printf(PRINT_GAP);

		if (item->content->type == NODE) {
			if (use_hash)
				print_hash((hash_t*)item->key);
			else
				print_str(item->key);

			printf(" : {\n");

			print_offset++;
			if (item->content->actions.print == NULL) 
				printf("NO PRINT\n");
			else
				item->content->actions.print(item->content);
			print_offset--;
			for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
			
			printf("}\n");
		}
		else {
			if (use_hash)
				print_hash((hash_t*)item->key);
			else
				print_str(item->key);

			printf(" : ");
			if (item->content->actions.print == NULL) 
				printf("NO PRINT");
			else
				item->content->actions.print(item->content);
			if (item->next != NULL)
				printf(",");
			printf("\n");
		}
		item = item->next;
	}

	return;
}

void json_print(json_t *item) {
	if (item == NULL) 
		return;

	printf("\n");
	if (item->content->type == LIST) {
		int is_null = use_hash ? print_hash((hash_t*)item->key) : print_str(item->key);

		if (is_null < 0) {
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
				if (use_hash)
					print_hash((hash_t*)item->key);
				else
					print_str(item->key);

				printf(" : {\n");

				print_offset++;
				if (item->content->actions.print == NULL) 
					printf("NO PRINT\n");
				else
					item->content->actions.print(item->content);
				print_offset--;
				for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
				
				printf("}\n");
			}
			else {
				print_hash((hash_t*)item->key);
				printf(" : ");
				if (item->content->actions.print == NULL) 
					printf("NO PRINT");
				else
					item->content->actions.print(item->content);
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
		if (item->content->actions.print == NULL) 
			printf("NO PRINT\n");
		else
			item->content->actions.print(item->content);
		printf("\n");
	}

	return;
}

/*
int json_for_each(void (*iter)(char*, value_t*), json_t *list) {
    if (list == NULL  iter == NULL  (json_get_type_of(list) != LIST && (json_get_type_of(list) != NODE))
        return -1;
    
    if (json_get_type_of(list) == NODE)
        list = node2list(list);
    
    json_t *item = list->next;
    while (item != NULL) {
        uint32_t *key = hash2str(item->key);
        if (key == NULL) {
            printf("JSON.forEach: couldn't extract key");
            item = item->next;
            continue;
        }
        iter(key, json_get_value_of(item));
        free(key);
        item = item->next;
    }
    
    return 1;
}

int json_modify(wchar_t *new_key, value_t *new_value, wchar_t *key, json_t *list) {
	list = json_check_list(list);
        return -1;
        
    hash = str2sha256(key);
    if (hash == NULL) {
        printf("JSON.modify: couldn't extract hash");
        return -1;
    }
    
    json_t *item = json_get(key, list);
    if (item == NULL) {
        printf("JSON.modify: couldn't find item with key '%ls'", key);
        return -1;
    }
    
    if (new_key != NULL) {
        json_t *check_key = list->next;
        uint32_t *new_hash = str2sha256(new_key);
        if (new_hash == NULL) {
            printf("JSON.modify: couldn't extract hash");
            free(hash);
            return -1;
        }
        while (check_key != NULL) {
            if(cmphash(hash, check_key->key)) {
                printf("JSON.modify: element with key '%ls' already exists", new_key);
                free(new_hash);
                free(hash);
                return -1;
            }
            check_key = check_key->next;
        }
        free(item->key);
        item->key = new_hash;
    }
    
    if (new_value != NULL) {
        if (json_get_type_of(item) == NODE) {
            json_remove_list((json_t*)json_get_value_of(item));
        }
        else {
            free(json_get_value_of(item));
        }
        item->content->value = new_value;
    }
    
    return 1;
}

void json_info(json_t *
*/
