#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <cjson/base.h>
#include <cjson/list.h>
#include <cjson/types.h>
#include <cjson/extra.h>
#include <cjson/config.h>
#include <cjson/sha256.h>

type_t jsonGetType(json_t *item) {
	return !item ? UNKNOWN : item->type;
}

info_t *jsonGetInfo(json_t *item) {
	return !item ? NULL : &item->info;
}

void *jsonGetValue(json_t *item) {
	if (!item)
		return NULL;

	return item->value;
}

static void printList(json_t *list, PrintFlags flags) {
	list = jsonCheckJsonList(list);
	if (!list)
		return;

	printf("{ ");
	print_offset++;

	json_t *item = list->next;
	while (item != NULL) {
		if (item->vtable->print != NULL) {
			if (jsonGetType(item) != HASH_NODE)
				for (size_t i = 0; i < print_offset; i++) printf(PRINT_GAP);

			item->vtable->print(item, flags);

			if (jsonGetType(item) != HASH_NODE) {
				if (item->next != NULL)
					printf("\033[0;37m,\033[0m");
			}
		}

		item = item->next;
	}

	print_offset--;
	printf("\b \n");
	for (size_t i = 0; i < print_offset; i++) printf(PRINT_GAP);
	printf("}");
}

static void printNode(json_t *node, PrintFlags flags) {
	if (!node || jsonGetType(node) != NODE)
		return;

	printf("\033[1;36m%s\033[0m: ", (char*)infoGetValue(infoFind("key", jsonGetInfo(node))));
	if (!node->value) {

		printf("\033[1;31m-\033[0m");
	}
	else {
		
		json_t* list = jsonCheckJsonList(node);
		
		if (list != NULL && list->vtable != NULL && list->vtable->print != NULL)
			list->vtable->print(list, flags);
	}

	return;
}

static void printHashNode(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != HASH_NODE) return;

	item = (json_t*)(item->value);
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
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind("type", jsonGetInfo(item))) != INT)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;35m%ld\033[0m", (char*)infoGetValue(infoFind("key", jsonGetInfo(item))), *(int64_t*)jsonGetValue(item));
}

/* print unsigned int */

static void printUint(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind("type", jsonGetInfo(item))) != UINT)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;35m%lu\033[0m", (char*)infoGetValue(infoFind("key", jsonGetInfo(item))), *(uint64_t*)jsonGetValue(item));
}

/* print char */

static void printChar(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind("type", jsonGetInfo(item))) != CHAR)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;32m\"%c\"\033[0m", (char*)infoGetValue(infoFind("key", jsonGetInfo(item))), *(char*)jsonGetValue(item));
}

static void printString(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind("type", jsonGetInfo(item))) != STRING)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;32m\"%s\"\033[0m", (char*)infoGetValue(infoFind("key", jsonGetInfo(item))), (char*)jsonGetValue(item));
}

static void freeList(json_t *list) {

	list = jsonCheckJsonList(list);
	
	if (!list)
		return;

	json_t *iter = list->next;
	while (iter != NULL) {
		jsonFree(iter);

		json_t *tmp = iter->next;
		iter->next = NULL;

		free(iter);
		iter = tmp;
	}

	free(list->value);

	list->value = NULL;
	list->next = NULL;

	return;
}

static void freeNode(json_t *node) {
	if (!node || jsonGetType(node) != NODE)
		return;

	jsonFree(node->value);
	
	free(node->value);
	node->value = NULL;

	return;
}

static void freeHashNode(json_t *hash_node) {
	if (!hash_node || jsonGetType(hash_node) != HASH_NODE)
		return;

	json_t *iter = hash_node->value;
	while (iter != NULL) {
		jsonFree(iter);

		json_t *tmp = iter->next;
		iter->next = NULL;

		free(iter);
		iter = tmp;
	}

	return;
}

static void freePlain(json_t *item) {
	if (!item || jsonGetType(item) != ITEM)
		return;

	free(jsonGetValue(item));
	item->value = NULL;
	item->vtable = NULL;
	
	return;
}

static void freeInt(json_t *item) {

	freePlain(item);

	return;
}

static void freeUint(json_t *item) {

	freePlain(item);

	return;
}

static void freeChar(json_t *item) {

	freePlain(item);

	return;
}

static void freeString(json_t *item) {

	freePlain(item);

	return;
}

json_ftable_t json_list_vtable		= { &printList, &freeList, NULL, NULL };
json_ftable_t json_node_vtable		= { &printNode, &freeNode, NULL, NULL };
json_ftable_t json_hash_node_vtable = { &printHashNode, &freeHashNode, NULL, NULL };
json_ftable_t json_int_vtable		= { &printInt, &freeInt, NULL, NULL };
json_ftable_t json_uint_vtable		= { &printUint, &freeUint, NULL, NULL };
json_ftable_t json_char_vtable		= { &printChar, &freeChar, NULL, NULL };
json_ftable_t json_string_vtable		= { &printString, &freeString, NULL, NULL };

json_t *jsonUnknown( void* const value, json_ftable_t* const vtable, json_t* const dest) {
	json_t *item = dest;

	if (!item) {

		item = (json_t*)malloc(sizeof(json_t));
		
		if (!item)
			return NULL;
	}

	item->info.array = NULL;
	item->info.length = 0;
	item->type = UNKNOWN;
	item->value = value;
	item->vtable = vtable;
	item->next = NULL;

	return item;
}


json_t *jsonHashNode() {

	json_t *new_item = jsonUnknown(NULL, &json_hash_node_vtable, NULL);

	if (!new_item)
		return NULL;

	info_t *info = infoNew(1, jsonGetInfo(new_item));

	new_item->type = HASH_NODE;

	size_t *pos = (size_t*)malloc(sizeof(size_t));
	*pos = rand_pos();

	infoAdd("pos", UINT, pos, info);

	return new_item;
}

json_t *jsonNew(json_t* const dest) {

	json_t *new_item = jsonUnknown(NULL, &json_list_vtable, dest);

	if (!new_item)
		return NULL;

	info_t *info = infoNew(1, jsonGetInfo(new_item));

	new_item->type = LIST;

	size_t *hash_length = (size_t*)malloc(sizeof(size_t));
	*hash_length = 0;

	for (size_t i = 0; i < default_hash_length; i++) {

		json_t *hash_node = jsonHashNode();
		
		if (hash_node != NULL && unshift(hash_node, new_item)) {

			(*hash_length)++;
		}
		else {
			
			jsonFree(hash_node);
			free(hash_node);

			hash_node = NULL;
		}
	}

	infoAdd("hash_length", UINT, hash_length, info);

	jsonQuickSortHashNodes(0, *hash_length-1, new_item);

	return new_item;
}

json_t *jsonNode(const char *key, json_t *value) {

	json_t *node = NULL;

	if (value != NULL && jsonGetType((json_t*)value) == LIST) {

		node = jsonUnknown(value, &json_node_vtable, NULL);

		if (!node) 
			return NULL;
	}
	else {

		json_t *list = jsonNew(NULL);

		if (!list)
			return NULL;

		node = jsonUnknown(list, &json_node_vtable, NULL);

		if (!node) {
			
			jsonFree(list);
			return NULL;
		}
	}

	hash_t *hash = str2sha256(key);

	if (!node || !key) {
		sha256Delete(hash);
		free(node);
	
		node = NULL;
		hash = NULL;

		return NULL;
	}

	char *tmp_key = clone2str(key);

	info_t *info = infoNew(2, jsonGetInfo(node));

	node->type = NODE;

	if (tmp_key != NULL)
		infoAdd("key", STRING, tmp_key, info);

	if (hash != NULL)
		infoAdd("hash", HASH, hash, info);

	return node;
}

/* int type */

json_t *jsonInt(const char *key, int64_t value) {
	
	int64_t *d_value = (int64_t*)malloc(sizeof(int64_t));
	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_int_vtable, NULL);
	hash_t *hash = str2sha256(key);

	if (!item || !key) {
		sha256Delete(hash);
		free(item);
		free(d_value);
	
		item = NULL;
		hash = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));

	item->type = ITEM;
	
	char *tmp_key = clone2str(key);

	if (tmp_key != NULL)
		infoAdd("key", STRING, tmp_key, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	*type = INT;

	infoAdd("type", UINT, type, info);

	if (hash != NULL)
		infoAdd("hash", HASH, hash, info);
	
	return item;
}

/* unsigned int type */

json_t *jsonUint(const char *key, uint64_t value) {

	uint64_t *d_value = (uint64_t*)malloc(sizeof(uint64_t));
	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_uint_vtable, NULL);
	hash_t *hash = str2sha256(key);

	if (!item || !key) {
		sha256Delete(hash);
		free(item);
		free(d_value);
	
		item = NULL;
		hash = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));

	item->type = ITEM;

	char *tmp_key = clone2str(key);

	if (tmp_key != NULL)
		infoAdd("key", STRING, tmp_key, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	*type = UINT;

	infoAdd("type", UINT, type, info);

	if (hash != NULL)
		infoAdd("hash", HASH, hash, info);

	return item;
}

/* char type */

json_t *jsonChar(const char *key, char value) {

	char *d_value = (char*)malloc(sizeof(char));
	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_char_vtable, NULL);
	hash_t *hash = str2sha256(key);

	if (!item || !key) {
		sha256Delete(hash);
		free(item);
		free(d_value);
	
		item = NULL;
		hash = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));
	
	item->type = ITEM;

	char *tmp_key = clone2str(key);

	if (tmp_key != NULL)
		infoAdd("key", STRING, tmp_key, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	*type = CHAR;

	infoAdd("type", UINT, type, info);

	if (hash != NULL)
		infoAdd("hash", HASH, hash, info);

	return item;
}

/* string type */

json_t *jsonString(const char *key, char *value) {

	char *d_value = clone2str(value);
	if (!d_value)
		return NULL;

	json_t *item = jsonUnknown(d_value, &json_string_vtable, NULL);
	hash_t *hash = str2sha256(key);

	if (!item || !key) {
		sha256Delete(hash);
		free(item);
		free(d_value);
	
		item = NULL;
		hash = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));
	
	item->type = ITEM;

	char *tmp_key = clone2str(key);

	if (tmp_key != NULL)
		infoAdd("key", STRING, tmp_key, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	*type = STRING;

	infoAdd("type", UINT, type, info);

	if (hash != NULL)
		infoAdd("hash", HASH, hash, info);

	return item;
}
