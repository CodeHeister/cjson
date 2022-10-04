#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "json_list.h"
#include "hash/sha256.h"

typedef union data {
	void *value; 
	unsigned int length;
} json_data;

struct json_t {
	uint32_t *key;
	data_type type;
	json_data data;
	json_t *next;
};

json_t *new_JSON() {
	json_t *new_list = (json_t*)malloc(sizeof(json_t));
	if (new_list == NULL) {
		return NULL;
	}

	new_list->key = NULL;
	new_list->type = LIST;
	new_list->data.length = 0;
	new_list->next = NULL;
	return new_list;
};

json_t *json_create(wchar_t *key, void *value, data_type type) {
	json_t *item = (json_t*)malloc(sizeof(json_t));
	if (item == NULL) {
		return NULL;
	}

	item->key = str2sha256(key);
	if (item->key == NULL) {
		free(item);
		return NULL;
	}

	item->type = type;
	item->data.value = value;
	item->next = NULL;
	return item;
}

json_t *json_get_by_index(unsigned int index, json_t *list) {
	if (list == NULL || list->type != LIST || index > list->data.length) {
		return NULL;
	}

	json_t *item = list->next;
	for (uint32_t i = 0; i < index; i++) {
		item = item->next;
	}
	return item;
}

json_t *json_get_by_key(wchar_t *key, json_t *list) {
	if (list == NULL || list->type != LIST) { 
		return NULL;
	}

	uint32_t *hash = str2sha256(key);
	if (hash == NULL) {
		return NULL;
	}

	json_t *item = list->next;
	while (item != NULL) {
		if (cmphash(item->key, hash)) {
			return item;
		}
		item = item->next;
	}
	return NULL;
}

int json_append(json_t *item, json_t *list) {
	if (item == NULL || list == NULL || list->type != LIST) { 
		return -1;
	}

	if (list->data.length != 0) { 
		json_t *last_item = json_get_by_index((list->data.length)-1, list);
		if (last_item->next != NULL) return -1;
		last_item->next = item;
	}
	else {
		if (list->next != NULL) return -1;
		list->next = item;
	}
	
	list->data.length++;
	return 1;
};

void *json_get_value_of(json_t* item) {
	if (item == NULL) return NULL;
	return item->data.value;
}

void *json_get_value(wchar_t* key, json_t* list) {
	if (list == NULL || list->type != LIST) { 
		return NULL;
	}
	
	json_t *item = json_get_by_key(key, list);
	if (item == NULL) return NULL;

	return json_get_value_of(item);
}

uint32_t json_length(json_t* list) {
	if (list == NULL || list->type != LIST) return 0;
	return list->data.length;
}

void *json_chain(wchar_t *keys, json_t *list) {
	if (list == NULL || list->type != LIST) { 
		return NULL;
	}

	uint32_t cur = 0, i = 0;
	json_t *node = list;

	while (keys[i] != '\0') {
		while (keys[i] != '.') {
			if (keys[i] == '\0') goto break_out;
			i++;
		}
		wchar_t *tmp_key = (wchar_t*)malloc( sizeof(wchar_t) * ((i-cur)+1) );
		if (tmp_key == NULL) return NULL;

		uint32_t tmp_cur = 0;
		while (keys[cur] != '.') {
			tmp_key[tmp_cur] = keys[cur];
			tmp_cur++;
			cur++;
		}
		tmp_key[tmp_cur+1] = '\0';
		
		node = (json_t*)json_get_by_key(tmp_key, node);
		if (node == NULL || node->type != NODE) return NULL;

		node = json_get_value_of(node);
		free(tmp_key);
		
		if (node == NULL || node->type != LIST) 
			return NULL;
		
		cur++;
		i++;
	}
break_out:
	wchar_t *tmp_key = (wchar_t*)malloc( sizeof(wchar_t) * ((i-cur)+1) );
	if (tmp_key == NULL) return NULL;

	uint32_t tmp_cur = 0;
	while (keys[cur] != '\0') {
		tmp_key[tmp_cur] = keys[cur];
		tmp_cur++;
		cur++;
	}
	tmp_key[tmp_cur+1] = '\0';
	
	node = json_get_by_key(tmp_key, node);
	free(tmp_key);
	
	if (node == NULL)
		return NULL;

	return node;
}


void *json_chain_value(wchar_t *keys, json_t *list) {
	return json_get_value_of(json_chain(keys, list));
}
