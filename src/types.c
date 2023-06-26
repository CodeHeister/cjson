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

static void printNode(json_t *item, PrintFlags flags) {
	if (!item || jsonGetType(item) != NODE) return;

	json_t *node = item->value;
	printf("\"%ls\" : ", (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(node))));
	if (!node->value) {
		printf("-");
	}
	else {
		json_t* list = node->value;
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

void jsonPrint(json_t *item, PrintFlags flags) {
	if (item != NULL && item->vtable != NULL)
		item->vtable->print(item, flags);
	printf("\n");

	return;
}

void freeList(json_t *list) {
	list = jsonCheckJsonList(list);
	if (!list)
		return;

	json_t *item = list->next;
	while (item != NULL) {
		jsonFree(item);

		item = item->next;

		free(item);
		item = NULL;
	}

	free(list->value);
	list->value = NULL;

	return;
}

void freeHashNode(json_t *item) {
	if (!item || jsonGetType(item) != HASH_NODE)
		return;

	json_t *iter = item->value;
	while (iter != NULL) {
		jsonFree(iter);

		iter = iter->next;
		
		free(iter);
		iter = NULL;
	}

	free(item->value);
	item->value = NULL;

	return;
}

json_ftable_t json_list_vtable = { &printList, &freeList, NULL };
json_ftable_t json_node_vtable = { &printNode, NULL, NULL };
json_ftable_t json_hash_node_vtable = { &printHashNode, &freeHashNode, NULL };
json_ftable_t json_int_vtable = { &printInt, NULL, NULL };
json_ftable_t json_uint_vtable = { &printUint, NULL, NULL };
json_ftable_t json_char_vtable = { &printChar, NULL, NULL };

json_t *jsonUnknown(void *value, json_ftable_t *vtable) {
	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (!item)
		return NULL;

	item->info.array = NULL;
	item->info.length = 0;
	item->type = UNKNOWN;
	item->value = value;
	item->vtable = vtable;
	item->next = NULL;

	return item;
}


json_t *jsonHashNode() {

	json_t *new_item = (json_t*)malloc(sizeof(json_t));

	if (!new_item)
		return NULL;

	info_t *info = infoNew(1, jsonGetInfo(new_item));

	new_item->value = NULL;
	new_item->type = HASH_NODE;
	new_item->vtable = &json_hash_node_vtable;
	new_item->next = NULL;

	size_t pos = rand_pos();
	infoAdd(L"pos", UINT, &pos, info);

	return new_item;
}

json_t *jsonNew(json_t *dest) {

	json_t *new_item = dest;

	if (!new_item) {
	
		new_item = (json_t*)malloc(sizeof(json_t));
		
		if (!new_item) {
			
			free(new_item);
			new_item = NULL;

			return NULL;
		}
	}

	info_t *info = infoNew(1, jsonGetInfo(new_item));

	new_item->type = LIST;
	new_item->value = NULL;
	new_item->vtable = &json_list_vtable;
	new_item->next = NULL;

	size_t hash_length = 0;

	for (size_t i = 0; i < default_hash_length; i++) {

		json_t *hash_node = jsonHashNode();
		
		if (hash_node != NULL && unshift(hash_node, new_item)) {

			hash_length++;
		}
		else {
			
			free(hash_node);
			hash_node = NULL;
		}
	}

	infoAdd(L"hash_length", UINT, &hash_length, info);

	jsonQuickSortHashNodes(0, hash_length-1, new_item);

	return new_item;
}

/* int type */

json_t *jsonInt(wchar_t *key, int64_t value) {
	
	json_t *item = jsonUnknown(NULL, &json_int_vtable);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		free(item);
		free(hash);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, &item->info);

	item->i64 = value;
	
	size_t count;
	for (count = 0; key[count] != L'\0'; count++);

	wchar_t *tmp_key = (wchar_t*)malloc(sizeof(wchar_t) * (count+1));
	for (count = 0; key[count] != L'\0'; count++)
		tmp_key[count] = key[count];
	tmp_key[count] = L'\0';

	infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = INT;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);
	

	item->type = ITEM;
	return item;
}

/* unsigned int type */

json_t *jsonUint(const wchar_t *key, uint64_t value) {

	json_t *item = jsonUnknown(NULL, &json_uint_vtable);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		free(item);
		free(hash);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, &item->info);

	item->u64 = value;

	size_t count;
	for (count = 0; key[count] != L'\0'; count++);

	wchar_t *tmp_key = (wchar_t*)malloc(sizeof(wchar_t) * (count+1));
	
	for (count = 0; key[count] != L'\0'; count++)
		tmp_key[count] = key[count];
	
	tmp_key[count] = L'\0';

	infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = UINT;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);

	item->type = ITEM;
	return item;
}

/* char type */

json_t *jsonChar(wchar_t *key, char value) {

	json_t *item = jsonUnknown(NULL, &json_char_vtable);
	hash_t *hash = wstr2sha256(key);

	if (!item || !key) {
		free(item);
		free(hash);
	
		item = NULL;
		hash = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, &item->info);
	
	item->c = value;

	size_t count;
	for (count = 0; key[count] != L'\0'; count++);

	wchar_t *tmp_key = (wchar_t*)malloc(sizeof(wchar_t) * (count+1));
	
	for (count = 0; key[count] != L'\0'; count++)
		tmp_key[count] = key[count];
	
	tmp_key[count] = L'\0';

	infoAdd(L"key", WSTRING, tmp_key, info);

	info_type_t type = CHAR;
	infoAdd(L"type", UINT, &type, info);

	if (hash != NULL)
		infoAdd(L"hash", HASH, hash, info);

	item->type = ITEM;
	return item;
}

