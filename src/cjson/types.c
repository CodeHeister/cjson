#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cjson.h"
#include "extra.h"
#include "config.h"

static void printList(json_t *list, PrintFlags flags) {
	list = JSON_check_json_list(list);
	if (!list)
		return;

	printf("{");
	print_offset++;

	json_t *item = list->next;
	while (item != NULL) {
		if (item->vtable->print != NULL) {
			if (JSON_type(item) != HASH_NODE)
				for (size_t i = 0; i < print_offset; i++) printf(PRINT_GAP);

			item->vtable->print(item, flags);

			if (JSON_type(item) != HASH_NODE) {
				if (item->next != NULL)
					printf(",");
			}
		}

		item = item->next;
	}

	print_offset--;
	for (size_t i = 0; i < print_offset; i++)
		printf(PRINT_GAP);
	printf("\b \n}");
}

static void printNode(json_t *item, PrintFlags flags) {
	if (!item || JSON_type(item) != NODE) return;

	node_t *node = item->item.node;
	printf("\"%ls\" : ", node->key);
	node->item->vtable->print(node->item, flags);

	return;
}

static void printHashNode(json_t *item, PrintFlags flags) {
	if (!item || JSON_type(item) != HASH_NODE) return;

	item = item->item.hash_node->item;
	while (item != NULL) {
		if (item->vtable->print != NULL) {
			printf("\n");
			for (size_t i = 0; i < print_offset; i++)
				printf(PRINT_GAP);
			item->vtable->print(item, flags);

			printf(",");
		}

		item = item->next;
	}

	return;
}

/* print int */

static void printInt(json_t *item, PrintFlags flags) {
	if (!item || JSON_type(item) != INT)
		return;

	printf("\"%ls\": %d", JSON_key(item), *(int*)JSON_value(item));
}

/* print unsigned int */

static void printUint(json_t *item, PrintFlags flags) {
	if (!item || JSON_type(item) != UINT)
		return;

	printf("\"%ls\": %u", JSON_key(item), *(unsigned int*)JSON_value(item));
}

/* print char */

static void printChar(json_t *item, PrintFlags flags) {
	if (!item || JSON_type(item) != CHAR)
		return;

	printf("\"%ls\": '%c'", JSON_key(item), *(char*)JSON_value(item));
}

void freeList(json_t *list) {
	list = JSON_check_json_list(list);
	if (!list) {
		errno = EINVAL;
		return;
	}

	json_t *item = list->next;
	while (item != NULL) {
		if (!item->vtable->free) {
			free(item->item.unknown);
			free(item);
		}
		else {
			item->vtable->free(item);
		}

		item = item->next;
	}
	free(list->item.json_list);
	free(list);

	return;
}

void freeHashNode(json_t *list) {
	if (!list || JSON_type(list) != HASH_NODE) {
		errno = EINVAL;
		return;
	}

	json_t *item = list->item.hash_node->item;
	while (item != NULL) {
		if (!item->vtable->free) {
			free(item->item.unknown);
			free(item);
		}
		else {
			item->vtable->free(item);
		}

		item = item->next;
	}
	free(list->item.hash_node);
	free(list);

	return;
}

json_ftable_t JSON_list_vtable = { &printList, &freeList, NULL };
json_ftable_t JSON_node_vtable = { &printNode, NULL, NULL };
json_ftable_t JSON_hashNode_vtable = { &printHashNode, &freeHashNode, NULL };
json_ftable_t JSON_int_vtable = { &printInt, NULL, NULL };
json_ftable_t JSON_uint_vtable = { &printUint, NULL, NULL };
json_ftable_t JSON_char_vtable = { &printChar, NULL, NULL };

json_t *JSON_node(wchar_t *key) {
	json_t *new_item = (json_t*)malloc(sizeof(json_t));
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	if (!new_item || !new_node || !key) {
		free(new_item);
		free(new_node);
		return NULL;
	}

	new_item->item.node = new_node;
	new_item->type = NODE;
	new_item->vtable = &JSON_node_vtable;
	new_item->next = NULL;

	new_node->key = clone_wstr(key);
	new_node->hash = str2sha256(key);
	new_node->item = NULL;
	return new_item;
}

/* create new list */

json_t *JSON_hash_node() {
	json_t *new_item = (json_t*)malloc(sizeof(json_t));
	hash_node_t *new_node = (hash_node_t*)malloc(sizeof(hash_node_t));
	if (!new_item || !new_node)
		return NULL;

	new_item->item.hash_node = new_node;
	new_item->type = HASH_NODE;
	new_item->vtable = &JSON_hashNode_vtable;
	new_item->next = NULL;

	new_node->pos = rand_pos();
	new_node->item = NULL;
	return new_item;
}

json_t *JSON_new() {
	json_t *new_item = (json_t*)malloc(sizeof(json_t));
	json_list_t *new_list = (json_list_t*)malloc(sizeof(json_list_t));
	if (!new_item || !new_list) {
		free(new_item);
		free(new_list);
		return NULL;
	}

	new_item->type = LIST;
	new_item->item.json_list = new_list;
	new_item->vtable = &JSON_list_vtable;
	new_item->next = NULL;

	new_list->length = 0;
	new_list->hash_length = 0;

	for (size_t i = 0; i < default_hash_length; i++)
		if (JSON_unshift(JSON_hash_node(), new_item) > 0)
			new_list->hash_length++;

	JSON_quick_sort_hash_nodes(new_item, 0, new_list->hash_length-1);
	return new_item;
}

json_t *JSON_unknown(wchar_t *key, void *value, json_ftable_t *vtable) {
	json_t *new_item = (json_t*)malloc(sizeof(json_t));
	json_item_t *item = (json_item_t*)malloc(sizeof(json_item_t));
	if (!new_item || !item)
		return NULL;

	new_item->type = UNKNOWN;
	new_item->item.json_item = item;
	new_item->vtable = vtable;

	item->key = clone_wstr(key);
	item->hash = str2sha256(key);
	item->value = value;

	return new_item;
}

/* int type */

json_t *JSON_int(wchar_t *key, int value) {
	int *new_int = (int*)malloc(sizeof(int));
	if (!new_int || !key) 
		return NULL;

	*new_int = value;
	json_t *item = JSON_unknown(key, (void*)new_int, &JSON_int_vtable);
	if (!item)
		return NULL;

	item->type = INT;
	return item;
}

/* unsigned int type */

json_t *JSON_uint(wchar_t *key, unsigned int value) {
	unsigned int *new_uint = (unsigned int*)malloc(sizeof(unsigned int));
	if (!new_uint || !key) 
		return NULL;

	*new_uint = value;
	json_t *item = JSON_unknown(key, (void*)new_uint, &JSON_uint_vtable);
	if (!item)
		return NULL;

	item->type = UINT;
	return item;
}

/* char type */

json_t *JSON_char(wchar_t *key, char value) {
	char *new_char = (char*)malloc(sizeof(char));
	if (!new_char || !key) 
		return NULL;

	*new_char = value;
	json_t *item = JSON_unknown(key, (void*)new_char, &JSON_char_vtable);
	if (!item)
		return NULL;

	item->type = CHAR;
	return item;
}

