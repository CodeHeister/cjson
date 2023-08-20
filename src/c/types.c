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

static bool first = 0;

type_t jsonType(json_t *item)
{
	return item ? item->type : UNKNOWN;
}

info_t *jsonInfo(json_t *item)
{
	return item ? &item->info : NULL;
}

void *jsonValue(json_t *item)
{
	return item ? item->value : NULL;
}

json_t *getList(void *key, json_t *list)
{
	if (!list || !key || jsonType(list) != LIST)
		return NULL;

	// init 
	hash_t *key_hash = str2sha256(key);
	json_t *hash_node = list->next;
	json_t *item = NULL;
	
	uint64_t key_mod = sha256Mod(key_hash, HASH_LIMIT);
	
	while (hash_node)
	{
		uint64_t *pos = (uint64_t*)infoGetValue("pos", jsonInfo(hash_node));

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

json_t *getNode(void *key, json_t *node)
{
	if (!key || !node || jsonType(node) != NODE)
		return 0;

	json_t *list = jsonCheckIfList(node);
	if (list)
		return jsonGet(key, list);

	return NULL;
}

bool addList(json_t *item, json_t *list)
{
	if (!list || !item || jsonType(list) != LIST)
		return 0;

	json_t *hash_node;
	
	hash_t *tmp_hash = (hash_t*)infoGetValue("hash", jsonInfo(item));
	char *tmp_key = NULL;
	
	if (!tmp_hash)
	{
		tmp_key = (char*)infoGetValue("key", jsonInfo(item));

		if (!tmp_key)
			tmp_hash = NULL;
		else
			tmp_hash = str2sha256(tmp_key);
	}

	if (!tmp_hash)
		return 0;

	uint64_t key_mod = sha256Mod(tmp_hash, HASH_LIMIT);

	if (tmp_key)
	{
		tmp_hash = NULL;
		sha256Delete(tmp_hash);
	}

	for (hash_node =  list->next;hash_node; hash_node = hash_node->next)
	{
		uint64_t *pos = (uint64_t*)infoGetValue("pos", jsonInfo(hash_node));
		if (pos && key_mod < *pos)
			break;
	}

	if (list->next && !hash_node)
		hash_node = list->next;

	if (!hash_node)
		return NULL;

	if (!hash_node->value)
	{
		hash_node->value = item;
	}
	else
	{
		json_t *check_item = (json_t*)(hash_node->value);
		hash_t *key_hash = (hash_t*)infoGetValue("hash", jsonInfo(item));
		tmp_key = NULL;
		
		if (!key_hash)
		{
			tmp_key = (char*)infoGetValue("key", jsonInfo(item));

			if (!tmp_key)
				key_hash = NULL;
			else
				key_hash = str2sha256(tmp_key);
		}

		if (!key_hash)
			return NULL;

		while (check_item)
		{
			if (sha256Compare((hash_t*)infoGetValue("hash", jsonInfo(check_item)), key_hash))
				break;
			
			check_item = check_item->next;
		}

		if (!check_item && check_item != item)
		{
			unshift(item, (json_t*)(hash_node->value));
		}
		else
		{	
			jsonFree(check_item);
			jsonMove(check_item, item);

			free(item);
			item = NULL;
		}

		if (tmp_key)
		{
			sha256Delete(key_hash);
			key_hash = NULL;
		}
	}

	return 1;
}

bool addNode(json_t *item, json_t *node)
{
	if (!item || !node || jsonType(node) != NODE)
		return 0;

	json_t *list = jsonCheckIfList(node);
	
	if (list)
		return jsonAdd(item, list);

	return 0;
}

static void printList(json_t *list, PrintFlags flags)
{
	if (!list || jsonType(list) != LIST)
		return;

	printf("{");
	print_offset++;

	first = 1;
	for (json_t *item = list->next; item; item = item->next)
		if (item->vtable && item->vtable->print)
			item->vtable->print(item, flags);
	print_offset--;

	printf("\n");
	for (uint64_t i = 0; i < print_offset; i++)
		printf(PRINT_GAP);
	printf("}");
}

static void printNode(json_t *node, PrintFlags flags)
{
	if (!node || jsonType(node) != NODE)
		return;

	printf("\033[1;36m%s\033[0m: ", (char*)infoGetValue("key", jsonInfo(node)));
	
	json_t* list = jsonCheckIfList(node);
	
	if (list && list->vtable && list->vtable->print)
	{
		list->vtable->print(list, flags);
		return;
	}

	printf("\033[1;31m-\033[0m");

	return;
}

static void printHashNode(json_t *hash_node, PrintFlags flags)
{
	if (!hash_node || jsonType(hash_node) != HASH_NODE)
		return;

	for (json_t *item = (json_t*)jsonValue(hash_node); item; item = item->next)
	{
		if (item->vtable && item->vtable->print)
		{
			if (first)
				first = 0;
			else
				printf(",");
			
			printf("\n");

			for (uint64_t i = 0; i < print_offset; i++)
				printf(PRINT_GAP);
			
			item->vtable->print(item, flags);
		}
	}

	return;
}

/* print int */

static void printInt(json_t *item, PrintFlags flags)
{
	if (!item || jsonType(item) != ITEM)
		return;

	info_type_t *type = (info_type_t*)infoGetValue("type", jsonInfo(item));
	if (!type || *type != INT)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;35m%ld\033[0m", (char*)infoGetValue("key", jsonInfo(item)), *(int64_t*)jsonValue(item));
}

/* print unsigned int */

static void printUint(json_t *item, PrintFlags flags)
{
	if (!item || jsonType(item) != ITEM)
		return;

	info_type_t *type = (info_type_t*)infoGetValue("type", jsonInfo(item));
	if (!type || *type != UINT)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;35m%lu\033[0m", (char*)infoGetValue("key", jsonInfo(item)), *(uint64_t*)jsonValue(item));
}

/* print char */

static void printChar(json_t *item, PrintFlags flags)
{
	if (!item || jsonType(item) != ITEM)
		return;

	info_type_t *type = (info_type_t*)infoGetValue("type", jsonInfo(item));
	if (!type || *type != CHAR)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;32m\"%c\"\033[0m", (char*)infoGetValue("key", jsonInfo(item)), *(char*)jsonValue(item));
}

static void printString(json_t *item, PrintFlags flags)
{
	if (!item || jsonType(item) != ITEM)
		return;

	info_type_t *type = (info_type_t*)infoGetValue("type", jsonInfo(item));
	if (!type || *type != STRING)
		return;

	printf("\033[1;36m%s\033[0m: \033[0;32m\"%s\"\033[0m", (char*)infoGetValue("key", jsonInfo(item)), (char*)jsonValue(item));
}

static void freeList(json_t *list)
{
	list = jsonCheckIfList(list);
	
	if (!list)
		return;

	for (json_t *iter = list->next; iter != NULL;)
	{
		jsonFree(iter);

		json_t *tmp = iter->next;
		iter->next = NULL;

		free(iter);
		iter = tmp;
	}

	free(jsonValue(list));

	list->value = NULL;
	list->next = NULL;

	return;
}

static void freeNode(json_t *node)
{
	if (!node || jsonType(node) != NODE)
		return;

	jsonFree(jsonValue(node));
	
	free(node->value);
	node->value = NULL;

	return;
}

static void freeHashNode(json_t *hash_node)
{
	if (!hash_node || jsonType(hash_node) != HASH_NODE)
		return;

	for (json_t *iter = hash_node->value; iter != NULL;)
	{
		jsonFree(iter);

		json_t *tmp = iter->next;
		iter->next = NULL;

		free(iter);
		iter = tmp;
	}

	return;
}

static void freePlain(json_t *item)
{
	if (!item || jsonType(item) != ITEM)
		return;

	free(jsonValue(item));
	item->value = NULL;
	item->vtable = NULL;
	
	return;
}

static void freeInt(json_t *item)
{

	freePlain(item);

	return;
}

static void freeUint(json_t *item)
{

	freePlain(item);

	return;
}

static void freeChar(json_t *item)
{

	freePlain(item);

	return;
}

static void freeString(json_t *item)
{

	freePlain(item);

	return;
}

json_ftable_t json_list_vtable		= { &printList, &freeList, NULL, NULL, &getList, &addList };
json_ftable_t json_node_vtable		= { &printNode, &freeNode, NULL, NULL, &getNode, &addNode };
json_ftable_t json_hash_node_vtable = { &printHashNode, &freeHashNode, NULL, NULL, NULL };
json_ftable_t json_int_vtable		= { &printInt, &freeInt, NULL, NULL, NULL, NULL };
json_ftable_t json_uint_vtable		= { &printUint, &freeUint, NULL, NULL, NULL, NULL };
json_ftable_t json_char_vtable		= { &printChar, &freeChar, NULL, NULL, NULL, NULL };
json_ftable_t json_string_vtable	= { &printString, &freeString, NULL, NULL, NULL, NULL };

json_t *jsonUnknown(void* const value, json_ftable_t* const vtable, json_t* const dest)
{
	json_t *item = dest;

	if (!item)
	{
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


json_t *jsonHashNode()
{
	json_t *new_item = jsonUnknown(NULL, &json_hash_node_vtable, NULL);

	if (!new_item)
		return NULL;

	info_t *info = infoNew(1, jsonInfo(new_item));

	new_item->type = HASH_NODE;

	uint64_t *pos = (uint64_t*)malloc(sizeof(uint64_t));
	*pos = rand_pos();

	infoAdd("pos", UINT, pos, info);

	return new_item;
}

json_t *jsonNew()
{
	json_t *new_item = jsonUnknown(NULL, &json_list_vtable, NULL);
	uint64_t *hash_length = (uint64_t*)malloc(sizeof(uint64_t));

	if (!new_item || !hash_length)
	{
		free(hash_length);
		jsonFree(new_item);
		free(new_item);

		new_item = NULL;
		hash_length = NULL;

		return NULL;
	}

	info_t *info = infoNew(1, jsonInfo(new_item));

	new_item->type = LIST;

	*hash_length = 0;

	for (uint64_t i = 0; i < default_hash_length; i++)
	{
		json_t *hash_node = jsonHashNode();
		
		if (hash_node && unshift(hash_node, new_item))
		{
			(*hash_length)++;
		}
		else
		{
			jsonFree(hash_node);
			free(hash_node);

			hash_node = NULL;
		}
	}

	infoAdd("hash_length", UINT, hash_length, info);

	jsonQuickSortHashNodes(0, *hash_length-1, new_item);

	return new_item;
}

json_t *jsonNode(const char *key, json_t *value)
{
	json_t *node = NULL;
	json_t *list = value;

	if (!list || jsonType(list) != LIST)
		list = jsonNew(NULL);

	if (!list)
		return NULL;

	node = jsonUnknown(list, &json_node_vtable, NULL);

	if (!node || !key)
	{
		if (!value && list)
			jsonDelete(list);

		free(node);
	
		node = NULL;

		return NULL;
	}


	info_t *info = infoNew(2, jsonInfo(node));

	node->type = NODE;

	char *tmp_key = clone2str(key);
	if (tmp_key)
		infoAdd("key", STRING, tmp_key, info);

	hash_t *hash = str2sha256(key);
	if (hash)
		infoAdd("hash", HASH, hash, info);

	return node;
}

/* int type */

json_t *jsonInt(const char *key, int64_t value)
{	
	int64_t *d_value = (int64_t*)malloc(sizeof(int64_t));

	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_int_vtable, NULL);
	if (!item || !key)
	{
		free(item);
		free(d_value);
	
		item = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonInfo(item));

	item->type = ITEM;
	
	char *tmp_key = clone2str(key);
	if (tmp_key)
		infoAdd("key", STRING, tmp_key, info);

	hash_t *hash = str2sha256(key);
	if (hash)
		infoAdd("hash", HASH, hash, info);
	
	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	if (type)
	{
		*type = INT;
		infoAdd("type", UINT, type, info);
	}

	return item;
}

/* unsigned int type */

json_t *jsonUint(const char *key, uint64_t value)
{
	uint64_t *d_value = (uint64_t*)malloc(sizeof(uint64_t));

	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_uint_vtable, NULL);
	if (!item || !key)
	{
		free(item);
		free(d_value);
	
		item = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonInfo(item));

	item->type = ITEM;

	char *tmp_key = clone2str(key);
	if (tmp_key)
		infoAdd("key", STRING, tmp_key, info);

	hash_t *hash = str2sha256(key);
	if (hash)
		infoAdd("hash", HASH, hash, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	if (type)
	{
		*type = UINT;
		infoAdd("type", UINT, type, info);
	}

	return item;
}

/* char type */

json_t *jsonChar(const char *key, char value)
{
	char *d_value = (char*)malloc(sizeof(char));
	
	if (!d_value)
		return NULL;

	*d_value = value;

	json_t *item = jsonUnknown(d_value, &json_char_vtable, NULL);
	if (!item || !key)
	{
		free(item);
		free(d_value);
	
		item = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonInfo(item));
	
	item->type = ITEM;

	char *tmp_key = clone2str(key);
	if (tmp_key)
		infoAdd("key", STRING, tmp_key, info);

	hash_t *hash = str2sha256(key);
	if (hash)
		infoAdd("hash", HASH, hash, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));

	if (type)
	{
		*type = CHAR;
		infoAdd("type", UINT, type, info);
	}

	return item;
}

/* string type */

json_t *jsonString(const char *key, char *value)
{
	char *d_value = clone2str(value);

	if (!d_value)
		return NULL;

	json_t *item = jsonUnknown(d_value, &json_string_vtable, NULL);
	if (!item || !key) {
		free(item);
		free(d_value);
	
		item = NULL;
		d_value = NULL;

		return NULL;
	}

	info_t *info = infoNew(3, jsonInfo(item));
	
	item->type = ITEM;

	char *tmp_key = clone2str(key);
	if (tmp_key)
		infoAdd("key", STRING, tmp_key, info);

	hash_t *hash = str2sha256(key);
	if (hash)
		infoAdd("hash", HASH, hash, info);

	info_type_t *type = (info_type_t*)malloc(sizeof(info_type_t));
	if (type)
	{
		*type = STRING;
		infoAdd("type", UINT, type, info);
	}

	return item;
}
