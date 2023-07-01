#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <cjson/types.h>
#include <cjson/sha256.h>

json_t *getLast(json_t *list)
{
	if (!list)
		return NULL;

	for (; list->next; list = list->next);

	return list;
}

bool append(json_t *item, json_t *list)
{
	if (!list || !item)
		return 0;

	json_t *last_item = getLast(list);
	if (!last_item)
		return 0;

	last_item->next = item;
	
	return 1;
}

bool unshift(json_t *item, json_t *list)
{
	if (!list || !item)
		return 0;

	json_t *first_item = list->next;
	list->next = item;

	json_t *item_last = getLast(item);
	if (!item_last)
		return 0;

	item_last->next = first_item;

	return 1;
}

void *infoHash(info_item_t *info)
{
	return (info) ? info->hash : NULL;
}

info_type_t infoType(info_item_t *info)
{
	return (info) ? info->type : BLANK;
}

void *infoValue(info_item_t *info)
{
	return (info) ? info->value : NULL;
}

bool infoSetItem(size_t i, hash_t *hash, info_type_t type, void *value, info_t *info)
{
	if (!info || i >= info->length)
		return 0;

	info->array[i].hash = hash;
	info->array[i].type = type;
	info->array[i].value = value;

	return 1;
}

bool infoCleanItem(info_item_t *item)
{
	if (!item) 
		return 0;

	item->hash = NULL;
	item->type = BLANK;
	item->value = NULL;

	return 1;
}

bool infoClean(size_t len, info_item_t *info)
{
	if (!info) 
		return 0;

	for (size_t i = 0; i < len; i++)
		infoCleanItem(&info[i]);

	return 1;
}

bool infoDump(info_t *info)
{
	if (!info)
		return 0;

	info->array = NULL;
	info->length = 0;

	return 1;
}

info_t *infoNew(size_t len, info_t *dest)
{

	info_t *info = dest;
	
	if (!info)
	{
		info = (info_t*)malloc(sizeof(info_t));
		
		if (!info)
			return NULL;
	}

	info->array = (len) ? (info_item_t*)malloc(sizeof(info_item_t) * len) : NULL;
	
	if (!info->array && len)
	{
		if (dest)
		{
			free(info);
			info = NULL;
		}
		
		return NULL;
	}

	info->length = len;

	infoClean(len, info->array);

	return info;
}

bool infoFreeItem(size_t index, info_t *info)
{
	if (!info || index >= info->length)
		return 0;
	
	sha256Delete(info->array[index].hash);
	free(info->array[index].value);

	info->array[index].hash = NULL;
	info->array[index].value = NULL;

	return 1;
}

bool infoFree(info_t *info)
{
	if (!info)
		return 0;

	for (size_t i = 0; i < info->length; i++)
		infoFreeItem(i, info);

	free(info->array);

	info->array = NULL;
	info->length = 0;

	return 1;
}

bool infoMove(info_t *src, info_t *dest)
{
	if (!src || !dest)
		return 0;

	dest->array = src->array;
	dest->length = src->length;

	infoDump(src);

	return 1;
}

bool infoIsItemBlank(info_item_t *item) {
	return (infoType(item) == BLANK && !infoHash(item) && !infoValue(item));
}


info_t *infoResize(int size, info_t *info)
{
	if (!info)
		return NULL;

	size_t tmp_len = 0;
	info_item_t *new_array = NULL;

	if (info->length+size > 0)
	{
		new_array = (info_item_t*)malloc(sizeof(info_item_t) * (info->length+size));

		if (!new_array)
			return NULL;

		tmp_len = info->length+size;

		infoClean(tmp_len, new_array);
	}
	else
	{
		infoFree(info);

		return info;
	}

	for (size_t i = 0; i < info->length; i++)
	{
		if (!infoIsItemBlank(&info->array[i]))
		{
			size_t j = 0;
			uint32_t mod = sha256Mod(infoHash(&info->array[i]), tmp_len);
			uint32_t modIndex = (j+mod)%tmp_len;
			
			for (;j < tmp_len && !infoIsItemBlank(&new_array[modIndex]); j++, modIndex = (i+mod)%tmp_len);

			if (infoIsItemBlank(&new_array[modIndex]))
			{
				new_array[modIndex].hash = infoHash(&info->array[i]);
				new_array[modIndex].type = infoType(&info->array[i]);
				new_array[modIndex].value = infoValue(&info->array[i]);
			}
			else
			{
				infoFreeItem(i, info);
			}
		}
	}

	free(info->array);

	info->array = new_array;
	info->length = tmp_len;

	return info;
}

info_t *infoAdd(char *key, info_type_t type, void *value, info_t *info)
{
	if (!info || !info->array)
		return NULL;

	hash_t *hash = str2sha256(key);
	if (key && !hash)
		return NULL;

	size_t i, j;
	uint32_t mod, modIndex;


	for (j = 0; j < 3; j++)
	{
		i = 0;
		mod = sha256Mod(hash, info->length);
		modIndex = (i+mod)%info->length;

		for (;i < info->length && !infoIsItemBlank(&info->array[modIndex]); i++, modIndex = (i+mod)%info->length);

		if (!infoIsItemBlank(&info->array[modIndex]))
			infoResize(1, info);
		else
			break;
	}

	if (j == 3)
		return NULL;

	infoSetItem(modIndex, hash, type, value, info);

	return info;
}

info_item_t *infoGet(char *key, info_t *info) {
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

		if (sha256Compare(infoHash(&info->array[modIndex]), hash)) {
		
			item = &info->array[modIndex];
			break;
		}
	}

	sha256Delete(hash);
	hash = NULL;

	return item;
}

void *infoGetValue(char *key, info_t *info) {
	return (info && key) ? infoValue(infoGet(key, info)) : NULL;
}

bool infoRemove(char *key, info_t *info) {
	if (!info || !key)
		return 0;
	
	info_item_t *item = infoGet(key, info);
	if (!item)
		return 0;

	sha256Delete(item->hash);
	if (item->type >= STRING)
		free(item->value);

	item->hash = NULL;
	item->value = NULL;

	infoCleanItem(item);

	return 1;
}

void infoPrintItem(info_item_t *item)
{
	if (!item)
		return;

	if (infoHash(item))
	{
		sha256Print(infoHash(item));
		
		printf("\ntype: %u\n", infoType(item));
		
		switch (infoType(item))
		{
			case INT:
				printf("value: %ld\n", *(int64_t*)infoValue(item));
				break;

			case UINT:
				printf("value: %lu\n", *(uint64_t*)infoValue(item));
				break;

			case CHAR:
				printf("value: '%c'\n", *(char*)infoValue(item));
				break;

			case WCHAR:
				printf("value: '%lc'\n", *(wchar_t*)infoValue(item));
				break;

			case STRING:
				printf("value: \"%s\"\n", (char*)infoValue(item));
				break;

			case WSTRING:
				printf("value: \"%ls\"\n", (wchar_t*)infoValue(item));
				break;

			case HASH:
				printf("value: ");
				sha256Print((hash_t*)infoValue(item));
				printf("\n");
				break;

			default:
				printf("value pointer: '%p'\n", infoValue(item));
		}
	}

	return;
}

void infoPrint(info_t *info)
{
	if (!info)
	{
		printf("No info");
		return;
	}

	printf("length: %lu\n\n", info->length); 

	info_item_t *arr = info->array;
	
	for (size_t i = 0; i < info->length; i++)
	{
		printf("%lu.\n", i);
		infoPrintItem(&arr[i]);
		printf("\n");
	}

	return;
}
