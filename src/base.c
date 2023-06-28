#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#include <cjson/types.h>
#include <cjson/sha256.h>

json_t *getLast(json_t *list) {
	if (!list)
		return NULL;

	json_t *item = list;

	while (item->next != NULL)
		item = item->next;

	return item;
}

bool append(json_t *item, json_t *list) {
	if (!list || !item)
		return 0;

	json_t *last_item = getLast(list);
	if (!last_item)
		return 0;

	last_item->next = item;
	
	return 1;
}

bool unshift(json_t *item, json_t *list) {
	if (!list || !item)
		return 0;

	json_t *next_item = list->next;
	list->next = item;

	json_t *last = getLast(item);
	last->next = next_item;

	return 1;
}

void *infoGetHash(info_item_t *info) {
	return (!info) ? NULL : info->hash;
}

info_type_t infoGetType(info_item_t *info) {
	return (!info) ? BLANK : info->type;
}

void *infoGetValue(info_item_t *info) {
	if (!info)
		return NULL;
	
	info_type_t type = infoGetType(info);

	switch (type) {
		case INT:
			return (void*)(&info->i64);
			break;
	
		case UINT:
			return (void*)(&info->u64);
			break;

		case DOUBLE:
			return (void*)(&info->f64);
			break;

		case CHAR:
			return (void*)(&info->c);
			break;

		case WCHAR:
			return (void*)(&info->wc);
			break;

		default:
			return info->value;
	}

	return NULL;
}

bool infoSetItem(size_t i, hash_t *hash, info_type_t type, void *value, info_t *info) {
	if (!info || i >= info->length)
		return 0;

	info->array[i].hash = hash;
	info->array[i].type = type;

	switch (type) {
		case INT:
			info->array[i].i64 = *(int64_t*)(value);
			break;
	
		case UINT:
			info->array[i].u64 = *(uint64_t*)(value);
			break;

		case DOUBLE:
			info->array[i].f64 = *(double*)(value);
			break;

		case CHAR:
			info->array[i].c = *(char*)(value);
			break;

		case WCHAR:
			info->array[i].wc = *(char32_t*)(value);
			break;

		default:
			info->array[i].value = value;
	}

	return 1;
}

bool infoCleanItem(info_item_t *item) {
	if (!item) 
		return 0;

	item->hash = NULL;
	item->type = BLANK;
	item->value = NULL;

	return 1;
}

bool infoClean(size_t len, info_item_t *info) {
	if (!info) 
		return 0;

	for (size_t i = 0; i < len; i++)
		infoCleanItem(&info[i]);

	return 1;
}

bool infoDump(info_t *info) {
	if (!info)
		return 0;

	info->array = NULL;
	info->length = 0;

	return 1;
}

info_t *infoNew(size_t len, info_t *dest) {

	info_t *info = dest;
	
	if (!info) { 

		info = (info_t*)malloc(sizeof(info_t));
		
		if (!info)
			return NULL;
	}

	info->array = !len ? NULL : (info_item_t*)malloc(sizeof(info_item_t) * len);
	
	if (!info->array && len != 0) {

		if (dest != NULL) {

			free(info);
			info = NULL;
		}
		
		return NULL;
	}

	info->length = len;

	infoClean(len, info->array);

	return info;
}

bool infoFreeItem(size_t index, info_t *info) {
	if (!info || index >= info->length)
		return 0;
	
	freeHash(info->array[index].hash);
	if (info->array[index].type >= STRING)
		free(info->array[index].value);

	info->array[index].hash = NULL;
	info->array[index].value = NULL;

	return 1;
}

bool infoFree(info_t *info) {
	if (!info)
		return 0;

	for (size_t i = 0; i < info->length; i++)
		infoFreeItem(i, info);

	free(info->array);

	info->array = NULL;
	info->length = 0;

	return 1;
}

bool infoMove(info_t *src, info_t *dest) {
	if (!src || !dest)
		return 0;

	dest->array = src->array;
	dest->length = src->length;

	infoDump(src);

	return 1;
}

info_t *infoResize(int size, info_t *info) {
	if (!info)
		return NULL;

	size_t tmp_len = 0;
	info_item_t *new_array = NULL;

	if (info->length+size > 0) {

		new_array = (info_item_t*)malloc(sizeof(info_item_t) * (info->length+size));

		if (!new_array)
			return NULL;

		tmp_len = info->length+size;

		infoClean(tmp_len, new_array);
	}
	else {

		infoFree(info);

		return info;
	}

	for (size_t i = 0; i < info->length; i++) {

		if (infoGetType(&info->array[i]) != BLANK && \
				(infoGetHash(&info->array[i]) != NULL || \
				 infoGetValue(&info->array[i]) != NULL)) {
			
			size_t j = 0;
			uint32_t mod = sha256Mod(infoGetHash(&info->array[i]), tmp_len);
			uint32_t modIndex = (j+mod)%tmp_len;
			
			while (j < tmp_len && \
					infoGetHash(&new_array[modIndex]) != NULL && \
					infoGetValue(&new_array[modIndex]) != NULL && \
					infoGetType(&new_array[modIndex]) != BLANK) {
		
				j++;
				modIndex = (i+mod)%tmp_len;
			}

			if (!infoGetHash(&new_array[modIndex]) && \
					!infoGetValue(&new_array[modIndex]) && \
					infoGetType(&new_array[modIndex]) == BLANK) {
				
				new_array[modIndex].hash = infoGetHash(&info->array[i]);
				new_array[modIndex].type = infoGetType(&info->array[i]);
				new_array[modIndex].value = infoGetValue(&info->array[i]);
			}
			else {
				
				infoFreeItem(i, info);
			}
		}
	}

	free(info->array);

	info->array = new_array;
	info->length = tmp_len;

	return info;
}

info_t *infoAdd(char *key, info_type_t type, void *value, info_t *info) {
	if (!info || !info->array)
		return NULL;

	hash_t *hash = str2sha256(key);
	if (key != NULL && !hash)
		return NULL;

	size_t i = 0;
	uint32_t mod = sha256Mod(hash, info->length);
	uint32_t modIndex = (i+mod)%info->length;

	while (i < info->length && \
			infoGetHash(&info->array[modIndex]) != NULL && \
			infoGetValue(&info->array[modIndex]) != NULL && \
			infoGetType(&info->array[modIndex]) != BLANK) {
		
		i++;
		modIndex = (i+mod)%info->length;
	}

	if (infoGetHash(&info->array[modIndex]) != NULL && \
			infoGetValue(&info->array[modIndex]) != NULL && \
			infoGetType(&info->array[modIndex]) != BLANK) {

		infoResize(1, info);

		i = 0;
		mod = sha256Mod(hash, info->length);
		modIndex = (i+mod)%info->length;

		while (i < info->length && \
				infoGetHash(&info->array[modIndex]) != NULL && \
				infoGetValue(&info->array[modIndex]) != NULL && \
				infoGetType(&info->array[modIndex]) != BLANK) {

			i++;
			modIndex = (i+mod)%info->length;
		}
	}

	infoSetItem(modIndex, hash, type, value, info);

	return info;
}

info_item_t *infoFind(char *key, info_t *info) {
	if (!info || !info->array)
		return NULL;

	hash_t *hash = str2sha256(key);
	if (!hash)
		return NULL;

	size_t i = 0;
	uint32_t mod = sha256Mod(hash, info->length);
	uint32_t modIndex = (i+mod)%info->length;

	info_item_t *item = NULL;

	for (i = 0; i < info->length; i++, modIndex = (i+mod)%info->length) {

		if (compareHash(infoGetHash(&info->array[modIndex]), hash)) {
		
			item = &info->array[modIndex];
			break;
		}
	}

	freeHash(hash);
	hash = NULL;

	return item;
}

bool infoRemove(char *key, info_t *info) {
	if (!info || !key)
		return 0;
	
	info_item_t *item = infoFind(key, info);
	if (!item)
		return 0;

	freeHash(item->hash);
	if (item->type >= STRING)
		free(item->value);

	item->hash = NULL;
	item->value = NULL;

	infoCleanItem(item);

	return 1;
}
