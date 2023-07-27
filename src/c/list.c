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

json_t *jsonGetByIndex(uint64_t index, json_t *list) {
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

	uint64_t *hash_length = (uint64_t*)(infoGetValue("hash_length", jsonInfo(list)));
	if (index >= *hash_length)
		return NULL;

	json_t *item = list;
	for (int i = 0; i <= index; i++)
		item = item->next;

	return jsonGetByIndex((uint64_t)index, list);
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
	uint64_t *pivot_pos = (uint64_t*)infoGetValue("pos", jsonInfo(pivot));

	int i = low-1;
	json_t *hash_node = jsonGetHashNodeByIndex(low, list);

	for (int j = low; j <= high-1; j++) {
		uint64_t *pos = (uint64_t*)infoGetValue("pos", jsonInfo(hash_node->next));
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

json_t *jsonGet(void *key, json_t *list)
{
	if (!list || !key)
		return 0;

	if (list->vtable && list->vtable->get)
		return list->vtable->get(key, list);

	return NULL;
}

void *jsonGetValue(char *key, json_t *list)
{
	return (key && list) ? jsonValue(jsonGet(key, list)) : NULL;
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

	if (key)
	{
		for (void *ptr = va_arg(args, void*); key; key = va_arg(args, char*), ptr = va_arg(args, void*))
		{
			if (ptr)
			{
				ptr = jsonGetValue(key, list);
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

bool jsonAdd(json_t *item, json_t *list)
{
	if (!list || !item)
		return 0;

	if (list->vtable && list->vtable->add)
		return list->vtable->add(item, list);

	return 0;
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
