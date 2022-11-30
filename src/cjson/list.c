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

/* get last item in list */

json_t *JSON_get_last(json_t *list) {
	if (!list)
		return NULL;

	json_t *item = list;
	json_t *last_item = NULL;
	while (item != NULL) {
		last_item = item;
		item = item->next;
	}

	return (item != NULL && !last_item) ? item : last_item;
}

/* append new item to list */

int JSON_append(json_t *item, json_t *list) {
	json_t *last_item = JSON_get_last(list);
	if (!last_item)
		return -1;

	last_item->next = item;
	
	return 1;
}

int JSON_unshift(json_t *item, json_t *list) {
	if (!list || !item)
		return -1;

	json_t *next_item = list->next;
	list->next = item;
	json_t *last = (!item->next) ? item : JSON_get_last(item);
	last->next = next_item;

	return 1;
}

json_t *JSON_get_hash_node_by_index(json_t *list, int index) {
    list = JSON_check_json_list(list);
	if (!list || index >= (int)(list->item.json_list->hash_length))
		return NULL;

	json_t *item = list;
	for (int i = 0; i <= index; i++)
		item = item->next;

	return item;
}

void JSON_swap_by_index(int i1, int i2, json_t *head) {
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

int JSON_partition (json_t *list, int low, int high) {

	json_t *pivot = JSON_get_hash_node_by_index(list, high);

	int i = low-1;
	json_t *hash_node = JSON_get_hash_node_by_index(list, low-1);

	for (int j = low; j <= high-1; j++) {
		if (hash_node->next->item.hash_node->pos < pivot->item.hash_node->pos) {
			i++;
			JSON_swap_by_index(i, j, list);
			hash_node = JSON_get_hash_node_by_index(list, j);
		}
		else {
			hash_node = hash_node->next;
		}
	}

	JSON_swap_by_index(i+1, high, list);
	return (i+1);
}

void JSON_quick_sort_hash_nodes(json_t *list, int low, int high) {
    list = JSON_check_json_list(list);
	if (!list)
		return;

	if (low < high) {
		int pi = JSON_partition(list, low, high);

		JSON_quick_sort_hash_nodes(list, low, pi - 1);

		JSON_quick_sort_hash_nodes(list, pi + 1, high);
	}
										
	return;
}

json_t *JSON_get(wchar_t *key, json_t *list) {
	list = JSON_check_json_list(list);
	if (!list || !key)
		return NULL;

	hash_t *key_hash = str2sha256(key);
	json_t *hash_node_item = list;
	json_t *item = NULL;
	
	uint32_t key_mod = hash_mod(key_hash, HASH_LIMIT);
	while (hash_node_item != NULL) {
		if (key_mod < hash_node_item->item.hash_node->pos) {
			json_t *tmp_item = hash_node_item->item.hash_node->item;
			while (tmp_item != NULL) {
				if (cmphash(tmp_item->item.json_item->hash, key_hash)) {
					item = tmp_item;
					break;
				}
				tmp_item = tmp_item->next;
			}
			break;
		}
		hash_node_item = hash_node_item->next;
	}

	if (!hash_node_item) {
		json_t *tmp_item = list->next->item.hash_node->item;
		while (tmp_item != NULL) {
			if (cmphash(tmp_item->item.json_item->hash, key_hash)) {
				item = tmp_item;
				break;
			}
			tmp_item = tmp_item->next;
		}
	}

	free(key_hash);
	return item;
}

int JSON_add(json_t *item, json_t *list) {
	list = JSON_check_json_list(list);
	if (!list || !item)
		return -1;

	json_t *hash_node_item = list;
	
	uint32_t key_mod = hash_mod(item->item.json_item->hash, HASH_LIMIT);
	while (hash_node_item != NULL) {
		if (key_mod < hash_node_item->item.hash_node->pos) {

			if (!hash_node_item->item.hash_node->item) {
				hash_node_item->item.hash_node->item = item;
				list->item.json_list->length++;
			}
			else {
				json_t *check_item = hash_node_item->item.hash_node->item;
				hash_t *key_hash = item->item.json_item->hash;

				while (check_item != NULL) {
					if (cmphash(check_item->item.json_item->hash, key_hash))
						break;
					check_item = check_item->next;
				}
				if (!check_item) {
					if (JSON_remove(item->item.json_item->key, list) > 0) {
						JSON_unshift(item, hash_node_item->item.hash_node->item);
						list->item.json_list->length++;
					}
				}
			}

			break;
		}
		hash_node_item = hash_node_item->next;
	}

	if (!hash_node_item) {
		if (!list->next->item.hash_node->item) {
			list->next->item.hash_node->item = item;
			list->item.json_list->length++;
		}
		else {
			json_t *check_item = list->next->item.hash_node->item;
			hash_t *key_hash = item->item.json_item->hash;

			while (check_item != NULL) {
				if (cmphash(check_item->item.json_item->hash, key_hash))
					break;
				check_item = check_item->next;
			}
			if (!check_item) {
				if (JSON_remove(item->item.json_item->key, list) > 0) {
					JSON_unshift(item, list->next->item.hash_node->item);
					list->item.json_list->length++;
				}
			}
		}
	}

	return 1;
}

void JSON_free(json_t *item) {
	if (!item)
		return;

	if (!item->vtable || !item->vtable->free) {
		free(item->item.unknown);
		free(item);
	}
	else {
		item->vtable->free(item);
	}

	return;
}

int JSON_remove(const wchar_t *key, json_t *list) {
	list = JSON_check_json_list(list);
	if (!list || !key) {
		errno = EINVAL;
		return -1;
	}

	hash_t *key_hash = str2sha256(key);
	json_t *hash_node_item = list;
	json_t *item = NULL;
	json_t *last_item = NULL;
	
	uint32_t key_mod = hash_mod(key_hash, HASH_LIMIT);
	while (hash_node_item != NULL) {
		if (key_mod < hash_node_item->item.hash_node->pos) {
			json_t *tmp_item = hash_node_item->item.hash_node->item;
			while (tmp_item != NULL) {
				if (cmphash(tmp_item->item.json_item->hash, key_hash)) {
					item = tmp_item;
					break;
				}
				last_item = tmp_item;
				tmp_item = tmp_item->next;
			}
			break;
		}
		hash_node_item = hash_node_item->next;
	}

	if (!hash_node_item) {
		json_t *tmp_item = list->next->item.hash_node->item;
		while (tmp_item != NULL) {
			if (cmphash(tmp_item->item.json_item->hash, key_hash)) {
				item = tmp_item;
				break;
			}
			last_item = tmp_item;
			tmp_item = tmp_item->next;
		}
	}

	free(key_hash);

	if (item != NULL) {
		if (!last_item)
			hash_node_item->item.hash_node->item = item->next;
		else
			last_item->next = item->next;
		JSON_free(item);
	}

	return 1;
}
