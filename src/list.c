#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <cjson/base.h>
#include <cjson/types.h>
#include <cjson/extra.h>

#include <chash/sha256.h>

json_t *jsonCheckJsonList(json_t *item) {
	return (item != NULL && jsonGetType(item) == NODE) ? (json_t*)(item->value) : item;
}

void jsonFree(json_t *item) {
	if (!item)
		return;

	if (!item->vtable || !item->vtable->free) {

		free(item->value);
		item->value = NULL;
	}
	else {
		
		item->vtable->free(item);
	}

	infoFree(jsonGetInfo(item));

	if (item->vtable != NULL)
		item->vtable = NULL;

	item->type = UNKNOWN;

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
    
	list = jsonCheckJsonList(list);

	if (!list)
		return NULL;

	size_t *hash_length = (size_t*)(infoGetValue(infoFind(L"hash_length", jsonGetInfo(list))));
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
	size_t *pivot_pos = (size_t*)(infoGetValue(infoFind(L"pos", jsonGetInfo(pivot))));

	int i = low-1;
	json_t *hash_node = jsonGetHashNodeByIndex(low, list);

	for (int j = low; j <= high-1; j++) {
		if (*(size_t*)(infoGetValue(infoFind(L"pos", jsonGetInfo(hash_node->next)))) < *pivot_pos) {
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

void jsonQuickSortHashNodes(int low, int high, json_t *list) {
 
	list = jsonCheckJsonList(list);
	
	if (!list)
		return;

	if (low < high) {
		int pi = jsonPartition(low, high, list);

		jsonQuickSortHashNodes(low, pi - 1, list);

		jsonQuickSortHashNodes(pi + 1, high, list);
	}
										
	return;
}

json_t *jsonGet(wchar_t *key, json_t *list) {

	list = jsonCheckJsonList(list);
	
	if (!list || !key)
		return NULL;

	// init 
	hash_t *key_hash = wstr2sha256(key);
	json_t *hash_node = list->next;
	json_t *item = NULL;
	
	uint32_t key_mod = sha256Mod(key_hash, HASH_LIMIT);
	
	while (hash_node != NULL) {

		if (key_mod < *(size_t*)infoGetValue(infoFind(L"pos", jsonGetInfo(hash_node))))
			break;

		hash_node = hash_node->next;
	}

	if (!hash_node)
		hash_node = list->next;

	json_t *tmp_item = (jsonGetType(hash_node) == HASH_NODE) ? (json_t*)jsonGetValue(hash_node) : NULL;
	
	while (tmp_item != NULL) {
		
		hash_t *tmp_hash = (hash_t*)infoGetValue(infoFind(L"hash", jsonGetInfo(tmp_item)));
		wchar_t *tmp_key = NULL;
		
		if (!tmp_hash) {
			
			tmp_key = (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(tmp_item)));

			if (!tmp_key) {

				tmp_hash = NULL;
			}
			else {

				tmp_hash = wstr2sha256(tmp_key);
			}
		}
		
		if (tmp_hash != NULL && compareHash(tmp_hash, key_hash)) {
		
			item = tmp_item;

			if (tmp_key != NULL) {

				freeHash(tmp_hash);
				tmp_hash = NULL;
			}

			break;
		}
		
		if (tmp_key != NULL) {

			freeHash(tmp_hash);
			tmp_hash = NULL;
		}

		tmp_item = tmp_item->next;
	}

	freeHash(key_hash);
	key_hash = NULL;

	return item;
}

bool jsonGetMultiple(json_t *list, ...) {
	if (!list)
		return 0;

	va_list args;
	va_start(args, list);
	 
	wchar_t *key = va_arg(args, wchar_t*);

	if (key != NULL) {

		for (void *ptr = va_arg(args, void*); key != NULL; key = va_arg(args, wchar_t*), ptr = va_arg(args, void*)) {

			if (ptr != NULL) {
				
				json_t *item = jsonGet(key, list);
				if (item != NULL) {
					
					info_type_t *type = (info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(item)));

					if (!type) {
						ptr = item->value;
					}
					else {
						switch (*type) {
							case INT:
								*(int64_t*)ptr = item->i64;
								break;
						
							case UINT:
								*(uint64_t*)ptr = item->u64;
								break;

							case DOUBLE:
								*(double*)ptr = item->f64;
								break;

							case CHAR:
								*(char*)ptr = item->c;
								break;

							case WCHAR:
								*(wchar_t*)ptr = item->wc;
								break;

							default:
								ptr = item->value;
						}
					}
				}
			}
		}
	}

	va_end(args);

	return 1;
}

bool jsonMove(json_t *src, json_t *dest) {
	if (!src || !dest)
		return 0;

	dest->type = src->type;
	dest->value = src->value;
	
	info_type_t *type = (info_type_t*)infoGetValue(infoFind(L"type", jsonGetInfo(src)));

	if (!type) {
		dest->value = src->value;
	}
	else {
		switch (*type) {
			case INT:
				dest->i64 = src->i64;
				break;
		
			case UINT:
				dest->u64 = src->u64;
				break;

			case DOUBLE:
				dest->f64 = src->f64;
				break;

			case CHAR:
				dest->c = src->c;
				break;

			case WCHAR:
				dest->wc = src->wc;
				break;

			default:
				dest->value = src->value;
		}
	}

	dest->vtable = src->vtable;

	src->value = NULL;
	
	infoMove(jsonGetInfo(src), jsonGetInfo(dest));
	jsonFree(src);

	src = NULL;

	return 1;
}

bool jsonAdd(json_t *item, json_t *list) {
	
	list = jsonCheckJsonList(list);
	
	if (!list || !item)
		return 0;

	json_t *hash_node = list->next;
	
	hash_t *tmp_hash = (hash_t*)infoGetValue(infoFind(L"hash", jsonGetInfo(item)));
	wchar_t *tmp_key = NULL;
	
	if (!tmp_hash) {
		
		tmp_key = (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(item)));

		if (!tmp_key) {

			tmp_hash = NULL;
		}
		else {

			tmp_hash = wstr2sha256(tmp_key);
		}
	}

	if (!tmp_hash)
		return NULL;

	uint32_t key_mod = sha256Mod(tmp_hash, HASH_LIMIT);

	if (tmp_key != NULL) {

		tmp_hash = NULL;
		freeHash(tmp_hash);
	}

	while (hash_node != NULL) {

		if (key_mod < *(size_t*)infoGetValue(infoFind(L"pos", jsonGetInfo(hash_node))))
			break;

		hash_node = hash_node->next;
	}

	hash_node = !hash_node ? list->next : hash_node;

	if (!hash_node->value) {

		hash_node->value = item;
	}
	else {

		json_t *check_item = (json_t*)(hash_node->value);
		hash_t *key_hash = (hash_t*)infoGetValue(infoFind(L"hash", jsonGetInfo(item)));
		tmp_key = NULL;
		
		if (!key_hash) {
			
			tmp_key = (wchar_t*)infoGetValue(infoFind(L"key", jsonGetInfo(item)));

			if (!tmp_key) {

				key_hash = NULL;
			}
			else {

				key_hash = wstr2sha256(tmp_key);
			}
		}

		if (!key_hash)
			return NULL;

		while (check_item != NULL) {
			
			if (compareHash((hash_t*)infoGetValue(infoFind(L"hash", jsonGetInfo(check_item))), key_hash))
				break;
			
			check_item = check_item->next;
		}
		if (!check_item) {

			unshift(item, (json_t*)(hash_node->value));
		}
		else {
			
			jsonMove(item, check_item);
			jsonFree(item);

			free(item);
			item = NULL;
		}

		if (tmp_key != NULL) {

			freeHash(key_hash);
			key_hash = NULL;
		}
	}

	return 1;
}
