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

type_t jsonGetType(json_t *item) {
	return !item ? UNKNOWN : item->type;
}

info_t *jsonGetInfo(json_t *item) {
	return !item ? NULL : &item->info;
}

void *jsonGetValue(json_t *item) {
	if (!item)
		return NULL;

	info_type_t *type = (info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(item)));

	if (!type) {
		return item->value;
	}
	else {
		switch (*type) {
			case INT:
				return (void*)(&item->i64);
				break;
		
			case UINT:
				return (void*)(&item->u64);
				break;

			case DOUBLE:
				return (void*)(&item->f64);
				break;

			case CHAR:
				return (void*)(&item->c);
				break;

			case WCHAR:
				return (void*)(&item->wc);
				break;

			default:
				return item->value;
		}
	}

	return NULL;
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
					printf(",");
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

	printf("\"%ls\" : ", (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(node))));
	if (!node->value) {

		printf("-");
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
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(item))) != INT)
		return;

	printf("\"%ls\": %ld", (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(item))), item->i64);
}

/* print unsigned int */

static void printUint(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(item))) != UINT)
		return;

	printf("\"%ls\": %u", (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(item))), *(unsigned int*)jsonGetValue(item));
}

/* print char */

static void printChar(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != ITEM || *(info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(item))) != CHAR)
		return;

	printf("\"%ls\": '%c'", (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(item))), *(char*)jsonGetValue(item));
}

void freeList(json_t *list) {

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

void freeNode(json_t *node) {
	if (!node || jsonGetType(node) != NODE)
		return;

	jsonFree(node->value);
	
	free(node->value);
	node->value = NULL;

	return;
}

void freeHashNode(json_t *hash_node) {
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

void freeInt(json_t *item) {
	if (!item || jsonGetType(item) != ITEM)
		return;

	item->i64 = 0;
	item->vtable = NULL;
	
	return;
}

void freeUint(json_t *item) {
	if (!item || jsonGetType(item) != ITEM)
		return;

	item->u64 = 0;
	item->vtable = NULL;
	
	return;
}

void freeChar(json_t *item) {
	if (!item || jsonGetType(item) != ITEM)
		return;

	item->c = 0;
	item->vtable = NULL;
	
	return;
}

json_ftable_t json_list_vtable = { &printList, &freeList, NULL };
json_ftable_t json_node_vtable = { &printNode, &freeNode, NULL };
json_ftable_t json_hash_node_vtable = { &printHashNode, &freeHashNode, NULL };
json_ftable_t json_int_vtable = { &printInt, &freeInt, NULL };
json_ftable_t json_uint_vtable = { &printUint, &freeUint, NULL };
json_ftable_t json_char_vtable = { &printChar, &freeChar, NULL };

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

	size_t pos = rand_pos();
	infoAdd(L"pos", UINT, &pos, info);

	return new_item;
}

json_t *jsonNew(json_t* const dest) {

	json_t *new_item = jsonUnknown(NULL, &json_list_vtable, dest);

	if (!new_item)
		return NULL;

	info_t *info = infoNew(1, jsonGetInfo(new_item));

	new_item->type = LIST;

	size_t hash_length = 0;

	for (size_t i = 0; i < default_hash_length; i++) {

		json_t *hash_node = jsonHashNode();
		
		if (hash_node != NULL && unshift(hash_node, new_item)) {

			hash_length++;
		}
		else {
			
			jsonFree(hash_node);
			free(hash_node);

			hash_node = NULL;
		}
	}

	infoAdd(L"hash_length", UINT, &hash_length, info);

	jsonQuickSortHashNodes(0, hash_length-1, new_item);

	return new_item;
}

json_t *jsonNode(const wchar_t *key, json_t *value) {

	json_t *node = NULL;

	if (value != NULL && jsonGetType((json_t*)value) == LIST) {

		node = jsonUnknown(value, &json_node_vtable, NULL);
	}
	else {

		json_t *list = jsonNew(NULL);

		if (!list)
			return NULL;

		node = jsonUnknown(list, &json_node_vtable, NULL);
	}
		

	if (!node) 
		return NULL;

	hash_t *hash = wstr2sha256(key);

	if (!node || !key) {
		freeHash(hash);
		free(node);
	
		node = NULL;
		hash = NULL;

		return NULL;
	}

	wchar_t *tmp_key = cloneWstr(key);

	info_t *info = infoNew(2, jsonGetInfo(node));

	node->type = NODE;

	if (tmp_key != NULL)
		infoAdd(L"key", WSTRING, tmp_key, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);

	return node;
}

/* int type */

json_t *jsonInt(const wchar_t *key, int64_t value) {
	
	json_t *item = jsonUnknown(NULL, &json_int_vtable, NULL);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		freeHash(hash);
		free(item);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));

	item->i64 = value;
	item->type = ITEM;
	
	wchar_t *tmp_key = cloneWstr(key);

	if (tmp_key != NULL)
		infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = INT;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);
	
	return item;
}

/* unsigned int type */

json_t *jsonUint(const wchar_t *key, uint64_t value) {

	json_t *item = jsonUnknown(NULL, &json_uint_vtable, NULL);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		freeHash(hash);
		free(item);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));

	item->u64 = value;
	item->type = ITEM;

	wchar_t *tmp_key = cloneWstr(key);

	if (tmp_key != NULL)
		infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = UINT;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);

	return item;
}

/* char type */

json_t *jsonChar(const wchar_t *key, char value) {

	json_t *item = jsonUnknown(NULL, &json_char_vtable, NULL);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		freeHash(hash);
		free(item);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonGetInfo(item));
	
	item->c = value;
	item->type = ITEM;

	wchar_t *tmp_key = cloneWstr(key);

	if (tmp_key != NULL)
		infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = CHAR;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);

	return item;
}

