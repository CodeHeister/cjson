#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <cjson/base.h>
#include <cjson/types.h>
#include <cjson/extra.h>
#include <cjson/sha256.h>

json_t *jsonCheckIfList(json_t *item) {
	if (!item)
		return NULL;

	type_t type = jsonType(item);

	if (type == NODE) {
		return (json_t*)(item->value);
	}
	else if (type == LIST) {
		return item;
	}

	return NULL;
}

void jsonFree(json_t *item) {
	if (!item)
		return;

	if (item->vtable && item->vtable->free) {

		item->vtable->free(item);
		item->value = NULL;
	}

	infoFree(jsonInfo(item));

	item->vtable = NULL;
	item->type = UNKNOWN;

	return;
}

void jsonDelete(json_t *item) {
	if (!item)
		return;

	jsonFree(item);
	free(item);

	return;
}

void jsonPrint(json_t *item, PrintFlags flags) {
	if (item != NULL && item->vtable != NULL)
		item->vtable->print(item, flags);
	printf("\n");

	return;
}

json_t *jsonGetByIndex(size_t index, json_t *list) {
	if (!list)
		return NULL;

	json_t *item = list;
	for (int i = 0; i <= index; i++)
		item = item->next;

	return item;
}

json_t *jsonGetHashNodeByIndex(int index, json_t *list) {
    
	list = jsonCheckIfList(list);

	if (!list)
		return NULL;

	size_t *hash_length = (size_t*)(infoGetValue("hash_length", jsonInfo(list)));
	if (index >= *hash_length)
		return NULL;

	json_t *item = list;
	for (int i = 0; i <= index; i++)
		item = item->next;

	return jsonGetByIndex((size_t)index, list);
}

void jsonSwapByIndex(int i1, int i2, json_t *head) {
    if (!head)
		return;

	json_t *preitem1 = NULL;
    json_t *preitem2 = NULL;
    json_t *item = head;

    for (int j = 0; item != NULL; j++) {
        if (j == i1)
            preitem1 = item;
          
        if (j == i2)
            preitem2 = item;
    
        item = item->next;
    }
    
    if (!preitem1 || !preitem2)
        return;

    json_t *tmp = preitem1->next;
    preitem1->next = preitem2->next;
    preitem2->next = tmp;

    tmp = preitem1->next->next;
    preitem1->next->next = preitem2->next->next;
    preitem2->next->next = tmp;
}

int jsonPartition(int low, int high, json_t *list) {

	json_t *pivot = jsonGetHashNodeByIndex(high, list);
	size_t *pivot_pos = (size_t*)infoGetValue("pos", jsonInfo(pivot));

	int i = low-1;
	json_t *hash_node = jsonGetHashNodeByIndex(low, list);

	for (int j = low; j <= high-1; j++) {
		size_t *pos = (size_t*)infoGetValue("pos", jsonInfo(hash_node->next));
		if (pos && *pos < *pivot_pos) {
			i++;
			jsonSwapByIndex(i, j, list);
			hash_node = jsonGetHashNodeByIndex(j, list);
		}
		else {
			hash_node = hash_node->next;
		}
	}

	jsonSwapByIndex(i+1, high, list);
	return (i+1);
}

void jsonQuickSortHashNodes(int low, int high, json_t *list)
{ 
	list = jsonCheckIfList(list);
	
	if (!list)
		return;

	if (low < high)
	{
		int pi = jsonPartition(low, high, list);

		jsonQuickSortHashNodes(low, pi - 1, list);

		jsonQuickSortHashNodes(pi + 1, high, list);
	}
										
	return;
}

json_t *jsonGet(const char *key, json_t *list)
{
	list = jsonCheckIfList(list);
	
	if (!list || !key)
		return NULL;

	// init 
	hash_t *key_hash = str2sha256(key);
	json_t *hash_node = list->next;
	json_t *item = NULL;
	
	uint32_t key_mod = sha256Mod(key_hash, HASH_LIMIT);
	
	while (hash_node)
	{
		size_t *pos = (size_t*)infoGetValue("pos", jsonInfo(hash_node));

		if (pos && key_mod < *pos)
			break;

		hash_node = hash_node->next;
	}

	if (list->next != NULL && !hash_node)
		hash_node = list->next;

	if (!hash_node)
	{
		sha256Delete(key_hash);
		key_hash = NULL;

		return NULL;
	}

	json_t *tmp_item = (jsonType(hash_node) == HASH_NODE) ? (json_t*)jsonValue(hash_node) : NULL;
	
	while (tmp_item)
	{
		hash_t *tmp_hash = (hash_t*)infoGetValue("hash", jsonInfo(tmp_item));
		char *tmp_key = NULL;
		
		if (!tmp_hash)
		{
			tmp_key = (char*)infoGetValue("key", jsonInfo(tmp_item));

			if (!tmp_key)
				tmp_hash = NULL;
			else
				tmp_hash = str2sha256(tmp_key);
		}
		
		if (tmp_hash && sha256Compare(tmp_hash, key_hash))
		{
			item = tmp_item;

			if (tmp_key)
			{
				sha256Delete(tmp_hash);
				tmp_hash = NULL;
			}

			break;
		}
		
		if (tmp_key)
		{
			sha256Delete(tmp_hash);
			tmp_hash = NULL;
		}

		tmp_item = tmp_item->next;
	}

	sha256Delete(key_hash);
	key_hash = NULL;

	return item;
}

void *jsonGetValue(char *key, json_t *list)
{
	return (key && list) ? jsonGet(key, list) : NULL;
}

bool jsonGetMultiple(json_t *list, ...) {
	if (!list)
		return 0;

	va_list args;
	va_start(args, list);
	 
	char *key = va_arg(args, char*);

	if (key)
	{
		for (void *ptr = va_arg(args, void*); key != NULL; key = va_arg(args, char*), ptr = va_arg(args, json_t*)) {

			if (ptr != NULL) {
				
				json_t *item = jsonGet(key, list);

				if (item != NULL)
						ptr = item;
			}
		}
	}

	va_end(args);

	return 1;
}

bool jsonGetMultipleValues(json_t *list, ...) {
	if (!list)
		return 0;

	va_list args;
	va_start(args, list);
	 
	char *key = va_arg(args, char*);

	if (key != NULL) {

		for (void *ptr = va_arg(args, void*); key != NULL; key = va_arg(args, char*), ptr = va_arg(args, void*)) {

			if (ptr != NULL) {
				
				json_t *item = jsonGet(key, list);

				if (item != NULL)
						ptr = jsonValue(item);
			}
		}
	}

	va_end(args);

	return 1;
}

bool jsonMove(json_t *dest, json_t *src) {
	if (!src || !dest)
		return 0;

	dest->type = src->type;
	dest->value = src->value;
	dest->vtable = src->vtable;

	src->value = NULL;
	
	infoMove(jsonInfo(src), jsonInfo(dest));
	jsonFree(src);

	return 1;
}

bool jsonAdd(json_t *item, json_t *list) {
	
	list = jsonCheckIfList(list);
	
	if (!list || !item)
		return 0;

	json_t *hash_node = list->next;
	
	hash_t *tmp_hash = (hash_t*)infoGetValue("hash", jsonInfo(item));
	char *tmp_key = NULL;
	
	if (!tmp_hash) {
		
		tmp_key = (char*)infoGetValue("key", jsonInfo(item));

		if (!tmp_key) {

			tmp_hash = NULL;
		}
		else {

			tmp_hash = str2sha256(tmp_key);
		}
	}

	if (!tmp_hash)
		return 0;

	uint32_t key_mod = sha256Mod(tmp_hash, HASH_LIMIT);

	if (tmp_key != NULL) {

		tmp_hash = NULL;
		sha256Delete(tmp_hash);
	}

	while (hash_node != NULL) {

		if (key_mod < *(size_t*)infoGetValue("pos", jsonInfo(hash_node)))
			break;

		hash_node = hash_node->next;
	}

	if (list->next != NULL && !hash_node)
		hash_node = list->next;

	if (!hash_node)
		return NULL;

	if (!hash_node->value) {

		hash_node->value = item;
	}
	else {

		json_t *check_item = (json_t*)(hash_node->value);
		hash_t *key_hash = (hash_t*)infoGetValue("hash", jsonInfo(item));
		tmp_key = NULL;
		
		if (!key_hash) {
			
			tmp_key = (char*)infoGetValue("key", jsonInfo(item));

			if (!tmp_key) {

				key_hash = NULL;
			}
			else {

				key_hash = str2sha256(tmp_key);
			}
		}

		if (!key_hash)
			return NULL;

		while (check_item != NULL) {
			
			if (sha256Compare((hash_t*)infoGetValue("hash", jsonInfo(check_item)), key_hash))
				break;
			
			check_item = check_item->next;
		}
		if (!check_item && check_item != item) {

			unshift(item, (json_t*)(hash_node->value));
		}
		else {
			
			jsonFree(check_item);
			jsonMove(check_item, item);

			free(item);
			item = NULL;
		}

		if (tmp_key != NULL) {

			sha256Delete(key_hash);
			key_hash = NULL;
		}
	}

	return 1;
}

json_t *jsonAddMultiple(json_t *item, ...) {
	if (!item)
		return NULL;

	va_list args;
	va_start(args, item);
	 
	json_t *list = item;

	if (list != NULL) {
		
		for (; list && jsonType(list) != LIST; list = va_arg(args, json_t*));
	}
	else {

		return NULL;
	}

	va_end(args);
	va_start(args, item);

	for (json_t *arg = item; arg != list; arg = va_arg(args, json_t*))
		if (jsonAdd(arg, list) != 1)
			return item;

	va_end(args);

	return list;
}
