#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "cjson.h"
#include "cjson_types.h"
#include "cjson_extra.h"
#include "../hash/sha256.h"

static size_t default_hash_nodes = 4;

/* output configuraions */

#define PRINT_GAP "  "

static uint8_t print_offset = 0; // offset counter for print function

/* content type for item */


struct value_t {
	void *value;		// value storage
	data_type type;		// abstract type storage
	struct {
		void (*print)(value_t*, uint32_t flags);	// print function
		void (*free)(void*);						// free function
		wchar_t *(*convert)(value_t*);				// json2string function
	};
};

typedef struct map_t map_t;
typedef struct map_item_t map_item_t;

struct map_item_t {
	size_t pos;
	json_t *ptr;
	map_item_t *next;
};

struct map_t {
	size_t length;
	size_t hash_nodes;
	map_item_t *hash_map;
};

/* base item type */

struct json_t {
	hash_t *hash;	// json hash
	wchar_t *key;	// json key
	value_t *content;	// item content
	json_t *next;		// next item pointer
};

void JSON_set_default_hash_nodes(size_t n) {
	default_hash_nodes = n;
	return;
}

/* get value from content */

void *JSON_value(value_t *content) {
	if (content == NULL) {
		errno = EINVAL;
		return NULL;
	}

	return content->value;
}

/* get type from content */

inline data_type JSON_type(value_t *content) {
	if (content == NULL) {
		errno = EINVAL;
		return NULL_JSON;
	}

	return content->type;
}

/* get content from item */

inline value_t *JSON_get_content_of(json_t* item) {
	if (!item)
		return NULL;

	return item->content;
}

/* get type from item */

inline data_type JSON_get_type_of(json_t *item) {
	return JSON_type(JSON_get_content_of(item));
}

/* get value from item */

void *JSON_get_value_of(json_t *item) {
	return JSON_value(JSON_get_content_of(item));
}

/* node in list translation */

static json_t *node2list(json_t *node) {
	if (!node || JSON_get_type_of(node) != NODE)
		return NULL;

	return (json_t*)(JSON_get_value_of(node));
}

/* check if list is valid (also node -> list) */

static json_t *JSON_check_list(json_t *list) {
	data_type type = JSON_get_type_of(list);
	if (!list || (type != LIST && type != NODE))
		return NULL;

	return (type == NODE) ? node2list(list) : list;
}

/* get content fomr key and list */

value_t *JSON_get_content(wchar_t* key, json_t* list) {
	return JSON_get_content_of(JSON_get(key, JSON_check_list(list)));
}

/* get type from key and list */

data_type JSON_get_type(wchar_t *key, json_t *list) {
	return JSON_type(JSON_get_content(key, JSON_check_list(list)));
}

/* get value from key and list */

void *JSON_get_value(wchar_t *key, json_t *list) {
	return JSON_value(JSON_get_content(key, JSON_check_list(list)));
}

/* get item in list from key */

json_t *JSON_get(wchar_t *key, json_t *list) {
	list = JSON_check_list(list);
	if (!list || !key) {
		errno = EINVAL;
		return NULL;
	}

	hash_t *hash = (hash_t*)str2sha256(key);

	if (!hash) {
		errno = ECANCELED;
		return NULL;
	}

	json_t *item = list->next;
	while (item != NULL) {
		if (cmphash(item->hash, hash) > 0)
			break;

		item = item->next;
	}

	free(hash);
	return item;
}

/* check if item with such name exists in list */

int JSON_key_exists(wchar_t *key, json_t *list) {
	return !JSON_get(key, list) ? 0 : 1;
}

size_t JSON_length(json_t* list) {
	list = JSON_check_list(list);
	if (list == NULL)
		return -1;

	map_t *info = (map_t*)JSON_get_value_of(list);
	if (info == NULL)
		return -1;

	return info->length;
}

/* list length recheck */

int JSON_recheck_length(json_t* list) {
	list = JSON_check_list(list);
	if (list == NULL)
		return -1;

	map_t *info = (map_t*)JSON_get_value_of(list);
	if (info == NULL)
		return -1;
	
	size_t length = 0;
	json_t *item = list->next;
	while (item != NULL) {
		length++;
		item = item->next;
	}

	if (info->length == length) {
		return 1;
	}
	else {
		info->length = length;
		return 0;
	}
}

/* item assembling */

json_t *JSON_create(wchar_t *key, value_t *content) {
	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (item == NULL)
		return NULL;

	item->key = clone_key(key);
	item->hash = str2sha256(key);

	item->content = content;
	item->next = NULL;
	return item;
}

/* free any item */

int JSON_free(json_t *item) {
	if (!item) {
		errno = EINVAL;
		return -1;
	}

	if (item->key != NULL) 
		free(item->key);

	value_t *content = JSON_get_content_of(item);
	void *value = JSON_value(content);
	if (content != NULL) {
		
		if (content->free == NULL)
			free(value);
		else
			content->free(value);
		
		free(item->content);
	}

	free(item);
	return 1;
}

/* free list item */

void JSON_free_list(void *value) {
	value = JSON_check_list(value);
	if (!value) {
		errno = EINVAL;
		return;
	}

	json_t *list = (json_t*)value;

	json_t *item = list->next;
	while (item != NULL) {
		printf("%d\n", JSON_free(item));

		item = item->next;
	}
	free(list->key);
	free(list);

	return;
}

/* free hash node */

static void JSON_free_hash_node(void *value) {
	if (!value) {
		errno = EINVAL;
		return;
	}
	json_t *hash_node = (json_t*)value;

	json_t *item = hash_node->next;
	while (item != NULL) {
		printf("%d\n", JSON_free(item));

		item = item->next;
	}
	free(hash_node);

	return;
}

/* get last item in list */

json_t *JSON_get_last(json_t *list) {
	list = JSON_check_list(list);
	if (!list) {
		errno = EINVAL;
		return NULL;
	}

	json_t *item = list;
	while (item->next != NULL) {
		item = item->next;
	}

	return item;
}

/* append new item to list */

int JSON_append(json_t *item, json_t *list) {
	list = JSON_check_list(list);
	if (!list) {
		errno = EINVAL;
		return -1;
	}

	if (JSON_key_exists(item->key, list) > 0) {
		errno = EPERM;
		return 0;
	}

	json_t *last_item = JSON_get_last(list);
	if (!last_item) {
		errno = EFAULT;
		return -1;
	}
	
	last_item->next = item;
	return 1;
};

/* add new item to list */

int JSON_add(wchar_t *key, value_t *content, json_t *list) {
	list = JSON_check_list(list);
	if (!list || !key || !content) {
		free(content->value);
		free(content);
		errno = EINVAL;
		return -1;
	}

	json_t *item = JSON_create(key, content);
	
	int append_status = JSON_append(item, list);
	if(append_status < 1) { 
		JSON_free(item);
		perror("JSON.append");
		return append_status;
	}

	if (item->key != NULL) {
		map_t *list_info = (map_t*)JSON_get_value_of(list);
		list_info->length++;
	}

	return 1;
}

/* remove an item from list */

int JSON_remove(const wchar_t *key, json_t *list) {
	list = JSON_check_list(list);
	if (list == NULL || key == NULL) {
		errno = EINVAL;
		return -1;
	}

	json_t *save_item = list;
	json_t *item = list->next;
	hash_t *hash = (hash_t*)str2sha256(key);

	while (item != NULL) {
		if (cmphash((hash_t*)item->key, (hash_t*)hash) == 1)
			break;

		save_item = item;
		item = item->next;
	}
	free(hash);

	if (item == NULL) {
		errno = EINVAL;
		return 0;
	}

	if (JSON_free(item) == 1) {
		save_item->next = item->next;
		map_t *list_info = (map_t*)JSON_get_value_of(list);
		list_info->length--;
		return 1;
	}
	else {
		errno = ECANCELED;
		return -1;
	}
}

/* advanced search item content */

value_t *JSON_chain_content(wchar_t *keys, json_t *list) {
	list = JSON_check_list(list);
	if (list == NULL || keys == NULL) {
		errno = EINVAL;
		return NULL;
	}

	return JSON_get_content_of(JSON_chain(keys, list));
}

data_type JSON_chain_type(wchar_t *keys, json_t *list) {
	list = JSON_check_list(list);
	if (list == NULL || keys == NULL)
		return NULL_JSON;

	return JSON_get_type_of(JSON_chain(keys, list));
}

void *JSON_chain_value(wchar_t *keys, json_t *list) {
	list = JSON_check_list(list);
	if (list == NULL || keys == NULL)
		return NULL;

	return JSON_get_value_of(JSON_chain(keys, list));
}

/* print int */

static void printInt(value_t *content, uint32_t flags) {
	printf("%d", *(int*)JSON_value(content));
}

/* print unsigned int */

static void printUint(value_t *content, uint32_t flags) {
	printf("%u", *(unsigned int*)JSON_value(content));
}

/* print char */

static void printChar(value_t *content, uint32_t flags) {
	printf("'%c'", *(char*)JSON_value(content));
}

static void printList(json_t *list, uint32_t flags) {
	list = JSON_check_list(list);
	if (list == NULL) return;

	printf("{\n");
	print_offset++;

	json_t *item = list->next;
	while (item != NULL) {
		if (item->key != NULL) {
			for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);

			printf("%ls : ", item->key);
		}
		if (item->content->print == NULL) 
			printf("NO PRINT");
		else
			item->content->print(item->content, flags);

		if (item->key != NULL) {
			if (item->next != NULL)
				printf(",");
			printf("\n");
		}

		item = item->next;
	}

	print_offset--;
	for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
	printf("}");
}

static void printNode(value_t *content, uint32_t flags) {
	if (content == NULL || (JSON_type(content) != NODE && JSON_type(content) != HASH_NODE)) return;

	printList((json_t*)JSON_value(content), flags);

	return;
}

static void printHashNode(value_t *content, uint32_t flags) {
	if (content == NULL)
		return;

	json_t *item = (json_t*)JSON_value(content);
	while (item != NULL) {
		if (item->key != NULL) {
			for (uint32_t i = 0; i < print_offset; i++) printf(PRINT_GAP);

				printf("%ls : ", item->key);
		}
		if (item->content->print == NULL) 
			printf("NO PRINT");
		else
			item->content->print(item->content, flags);

		if (item->key != NULL) {
			if (item->next != NULL)
				printf(",");
			printf("\n");
		}

		item = item->next;
	}

	return;
}

void JSON_print(json_t *item, uint32_t flags) {
	if (!item) 
		return;

	if (item->key != NULL)
		printf("%ls : ", item->key);

	if (item->content->print == NULL) {
		printf("NO PRINT\n");
	}
	else {
		if (JSON_get_type_of(item) == LIST)
			item->content->print(item, flags);
		else
			item->content->print(item->content, flags);
	}

	printf("\n");
	return;
}

/* unknown type */

value_t *JSON_unknown(void *value, void (*print)(value_t*, uint32_t), void (*free)(void*), wchar_t *(*convert)(value_t*)) {
	value_t *new_unknown = (value_t*)malloc(sizeof(value_t));
	if (new_unknown == NULL) 
		return NULL;

	new_unknown->type = UNKNOWN;
	new_unknown->value = value;
	new_unknown->print = print;
	new_unknown->free = free;
	new_unknown->convert = convert;

	return new_unknown;
}

value_t *JSON_hash_node() {
	value_t *item = JSON_unknown(NULL, &printHashNode, &JSON_free_hash_node, NULL);
	if (item == NULL)
		return NULL;

	item->type = HASH_NODE;
	return item;
}

/* create new list */

json_t *JSON_new() {
	json_t *new_list = (json_t*)malloc(sizeof(json_t));
	map_t *list_info = (map_t*)malloc(sizeof(map_t));
	value_t *list_content = JSON_unknown(list_info, &printList, NULL, NULL);
	if (new_list == NULL || list_info == NULL || list_content == NULL) {
		free(list_content);
		free(list_info);
		free(new_list);
		return NULL;
	}

	list_info->length = 0;
	list_info->hash_nodes = 0;
	list_info->hash_map = NULL;

	new_list->key = NULL;

	new_list->content = (value_t*)malloc(sizeof(value_t));
	if (new_list->content == NULL) {
		free(list_content);
		free(list_info);
		free(new_list);
		return NULL;
	}

	new_list->content = list_content;
	new_list->content->type = LIST;
	new_list->next = NULL;

	json_t *hash_node = new_list;
	for (size_t i = 0; i < default_hash_nodes; i++) {
		if (JSON_append(JSON_create(NULL, JSON_hash_node()), new_list) < 1) {
			perror("JSON.append");
		}
		else {
			map_item_t *hash_map_item = list_info->hash_map;
			while (hash_map_item != NULL) 
				hash_map_item = hash_map_item->next;
			hash_map_item = (map_item_t*)malloc(sizeof(map_item_t));

			hash_node = hash_node->next;
			list_info->hash_nodes++;

			hash_map_item->pos = rand_pos();
			hash_map_item->ptr = hash_node;
		}
	}

	return new_list;
};

/* int type */

value_t *JSON_int(int value) {
	int *new_int = (int*)malloc(sizeof(int));
	if (new_int == NULL) 
		return NULL;

	*new_int = value;
	value_t *item = JSON_unknown((void*)new_int, &printInt, NULL, NULL);
	if (item == NULL)
		return NULL;

	item->type = INT;
	return item;
}

/* unsigned int type */

value_t *JSON_uint(unsigned int value) {
	unsigned int *new_uint = (unsigned int*)malloc(sizeof(unsigned int));
	if (new_uint == NULL) 
		return NULL;

	*new_uint = value;
	value_t *item = JSON_unknown((void*)new_uint, &printUint, NULL, NULL);
	if (item == NULL)
		return NULL;

	item->type = UINT;
	return item;
}

/* char type */

value_t *JSON_char(char value) {
	char *new_char = (char*)malloc(sizeof(char));
	if (new_char == NULL) 
		return NULL;

	*new_char = value;
	value_t *item = JSON_unknown((void*)new_char, &printChar, NULL, NULL);
	if (item == NULL)
		return NULL;

	item->type = CHAR;
	return item;
}

/* node (list in list) type */

value_t *JSON_node() {
	value_t *item = JSON_unknown((void*)JSON_new(), &printNode, &JSON_free_list, NULL);
	if (item == NULL)
		return NULL;

	item->type = NODE;
	return item;
}

void JSON_info(json_t *item, info_flags flags) {
	if (item == NULL)
		return;

	if (flags & SHOW_KEY) {
		if (!item->key)
			printf("\033[36mKEY\033[39m : \033[31mno key\033[39m");
		else
			printf("\033[36mKEY\033[39m : %ls", item->key);

		printf("\n\n");
	}

	if (flags & SHOW_HASH) {
		print_hash(item->hash);
		printf("\n\n");
	}

	if (flags & SHOW_TYPE)
		printf("\033[36mTYPE\033[39m : %d\n\n", JSON_get_type_of(item));

	if (flags & SHOW_LENGTH) {
		data_type type = JSON_get_type_of(item);
		printf("\033[36mLENGTH\033[39m : ");
		if (type == INT) {
			printf("%d", (int)ceil(log10(*(int*)JSON_get_value_of(item))));
		}
		else if (type == UINT) {
			printf("%d", (int)ceil(log10(*(unsigned int*)JSON_get_value_of(item))));
		}
		else if (type == LIST || type == NODE) {
			printf("%lu", JSON_length(item));
		}
		else {
			printf("\033[31mundefined\033[39m");
		}
		printf("\n\n");
	}

	if (flags & SHOW_VALUE) {
		printf("\033[36mVALUE\033[39m :\n\n");
		JSON_print(item, 0);
		printf("\n\n");
	}

	if (flags & SHOW_TREE) {
		json_t *list = JSON_check_list(item);
		
		printf("\033[36mTREE\033[39m :\n\n");
		if (!list) {
			printf("\033[31mnot a list or node\033[39m");
		}
		else {
			printf("\033[34mjson-obj");
		}
		printf("\n\n");
	}

	return;
}

/* ----------------- */

json_t *JSON_chain(wchar_t *keys, json_t *list) {
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
		
		node = JSON_get(tmp_key, node);
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
	
	node = JSON_get(tmp_key, node);
	free(tmp_key);

	return node;
}

int JSON_for_each(void (*iter)(json_t*, wchar_t*, value_t*), json_t *list) {
	list = JSON_check_list(list);
	if (list == NULL || iter == NULL)
		return -1;
    
    json_t *item = list->next;
    while (item != NULL) {
		if (item != NULL && item->key != NULL)
			iter(item, item->key, JSON_get_content_of(item));

        item = item->next;
    }
    
    return 1;
}

/*
int JSON_partition (map_t *hash_map, int low, int high) {

    // pivot (Element to be placed at right position)
	map_t *pivot = hash_map[high];

	int i = low-1;  // Index of smaller element and indicates the 
	// right position of pivot found so far

	for (int j = low; j <= high-1; j++) {

		 // If current element is smaller than the pivot
		if (hash_map[j] < pivot) {
			i++;    // increment index of smaller element
			int tmp = hash_map[i];
			hash_map[i] = hash_map[j];
			hash_map[j] = tmp;
		}
	}

	int tmp = hash_map[i + 1];
	hash_map[i + 1] = hash_map[high];
	hash_map[high] = tmp;
	return (i+1);
}

void JSON_quick_sort(map_t *hash_map, int low, int high) {
    if (low < high) {

        // pi is partitioning index, arr[pi] is now at right place

        int pi = JSON_partition(hash_map, low, high);

        JSON_quickSort(hash_map, low, pi - 1);  // Before pi

        JSON_quickSort(hash_map, pi + 1, high); // After pi

    }

}
*/
/*

int JSON_modify(wchar_t *new_key, value_t *new_value, wchar_t *key, json_t *list) {
	list = JSON_check_list(list);
        return -1;
        
    hash = str2sha256(key);
    if (hash == NULL) {
        printf("JSON.modify: couldn't extract hash");
        return -1;
    }
    
    json_t *item = JSON_get(key, list);
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
        if (JSON_get_type_of(item) == NODE) {
            JSON_remove_list((json_t*)JSON_get_value_of(item));
        }
        else {
            free(JSON_get_value_of(item));
        }
        item->content->value = new_value;
    }
    
    return 1;
}*/
