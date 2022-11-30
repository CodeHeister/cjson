#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "cjson.h"
#include "extra.h"
#include "config.h"

DataType JSON_type(json_t *item) {
	if (!item) {
		errno = EINVAL;
		return NULL_JSON;
	}

	return item->type;
}

void *JSON_value(json_t *item) {
	DataType type = JSON_type(item);
	if (!item || type < INT) {
		errno = EINVAL;
		return NULL_JSON;
	}

	return item->item.json_item->value;
}

wchar_t *JSON_key(json_t *item) {
	DataType type = JSON_type(item);
	if (!item || type < NODE) {
		errno = EINVAL;
		return NULL;
	}

	return item->item.json_item->key;
}

hash_t *JSON_hash(json_t *item) {
	DataType type = JSON_type(item);
	if (!item || type < NODE) {
		errno = EINVAL;
		return NULL;
	}

	return item->item.json_item->hash;
}

json_t *node2list(json_t *node) {
	return node->item.node->item;
}

json_t *JSON_check_list(json_t *item) {
	DataType type = JSON_type(item);
	if (!item || (type != LIST && type != LINKED_LIST && type != NODE && type != LL_NODE && type != HASH_NODE))
		return NULL;

	if (type == NODE || type == LL_NODE)
		item = node2list(item);

	return item;
}

json_t *JSON_check_json_list(json_t *item) {
	DataType type = JSON_type(item);
	if (!item || (type != LIST && type != NODE))
		return NULL;

	if (type == NODE)
		item = node2list(item);

	return item;
}

void JSON_print(json_t *item, PrintFlags flags) {
	item->vtable->print(item, flags);
	printf("\n");
	return;
}

size_t JSON_length(json_t *item) {
	item = JSON_check_list(item);
	if (!item)
		return 0;

	DataType type = JSON_type(item);
	if (type == LIST)
		return item->item.json_list->length;
	else if (type == LINKED_LIST)
		return item->item.linked_list->length;

	return 0;
}

void JSON_info(json_t *item, InfoFlags flags) {
	if (!item) {
		printf("NULL item\n");
		return;
	}

	DataType type = JSON_type(item);
	if (flags & SHOW_KEY) {
		wchar_t *key = JSON_key(item);
		if (!key)
			printf("\033[36mKEY\033[39m : \033[31mno key\033[39m");
		else
			printf("\033[36mKEY\033[39m : %ls", key);

		printf("\n\n");
	}

	if (flags & SHOW_HASH) {
		hash_t *hash = JSON_hash(item);
		if (!hash)
			printf("\033[36mSHA256\033[39m : \033[31mno hash\033[39m");
		else
			print_hash(JSON_hash(item));

		printf("\n\n");
	}

	if (flags & SHOW_TYPE)
		printf("\033[36mTYPE\033[39m : %d\n\n", JSON_type(item));

	if (flags & SHOW_LENGTH) {
		printf("\033[36mLENGTH\033[39m : ");
		if (type == INT) {
			printf("%d", (int)ceil(log10(*(int*)JSON_value(item))));
		}
		else if (type == UINT) {
			printf("%d", (int)ceil(log10(*(unsigned int*)JSON_value(item))));
		}
		else if (type == LIST || type == LINKED_LIST || type == NODE || type == LL_NODE) {
			printf("%lu", JSON_length(item));
		}
		else {
			printf("\033[31mundefined\033[39m");
		}
		printf("\n\n");
	}

	if (flags & SHOW_VALUE) {
		printf("\033[36mVALUE\033[39m :\n\n");
		JSON_print(item, 0);
		printf("\n\n");
	}
/*
	if (flags & SHOW_TREE) {
		json_t *list = JSON_check_list(item);
		
		printf("\033[36mTREE\033[39m :\n\n");
		if (!list) {
			printf("\033[31mnot a list or node\033[39m");
		}
		else {
			printf("\033[34mjson-obj");
		}
		printf("\n\n");
	}
*/
	return;
}
