#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

#define itemType int64_t
#define Item (item_t*)(malloc(sizeof(item_t)))
#define Array (da_t*)(malloc(sizeof(da_t)))

typedef struct dynamic_array da_t;
typedef struct array_item item_t;
item_t *get(da_t *array, int i);

struct array_item {
	itemType value;
	item_t *next;
};

struct dynamic_array {
	int length;
	item_t *(*get)(da_t*, int);
	int (*append)(da_t*, int, ...);
	da_t *(*splice)(da_t*, int, int);
	int (*remove)(da_t*, int, int);
	int (*insertBefore)(da_t*, int, int, ...);
	int (*insertAfter)(da_t*, int, int, ...);
	item_t *next;
};

item_t *get_item(da_t *array, int index);
int append_item(da_t *array, int count, ...);
da_t *splice_items(da_t *array, int index, int count);
int remove_items(da_t *array, int index, int count);
int insertBefore_item(da_t *array, int index, int count, ...);
int insertAfter_item(da_t *array, int index, int count, ...);

da_t DefaultArray = { 0, &get_item, &append_item, &splice_items, &remove_items, &insertBefore_item, &insertAfter_item, NULL };

item_t *get_item(da_t *array, int index) {
	if (index >= array->length) {
		printf("%s:%d: %d is out of range\n", __FILE__, __LINE__, index);
		return NULL;
	}
	item_t *item = array->next;
	int i = 0;
	while (i < index) {
		item = item->next;
		i++;
	}
	return item;
}

int append_item(da_t *array, int count, ...) {
	va_list ap;
	itemType i;
	
	va_start(ap, count);
	item_t *item = array->get(array, (array->length)-1);
	for (int i = 0; i < count; i++) {
		itemType value = va_arg(ap, itemType);
		if (item == NULL) { 
			array->next = Item;
			item = array->next;
		}
		else {
			item->next = Item;
			item = item->next;
		}
		if (item == NULL) {
			printf("%s:%d: Couldn't add new item\n", __FILE__, __LINE__);
			return 1;
		}
		item->value = value;
		array->length++;
	}
	item->next = NULL;
	va_end(ap);
	return 0;
}

da_t *splice_items(da_t *array, int index, int count) {
	item_t *item = array->next;
	if (index+count > array->length) {
		printf("%s:%d: %d is out of range\n", __FILE__, __LINE__, index);
		return NULL;
	}
	if (count == 0) {
		printf("%s:%d: Zero count splice\n", __FILE__, __LINE__);
		return NULL;
	}
	da_t *new_array = malloc(sizeof(da_t));
	*new_array = DefaultArray;
	new_array->length = count;
	if (index != 0) {
		item_t *item = array->get(array, index-1);
		new_array->next = item->next;
		item_t *last_item = array->get(array, index+count-1);
		item->next = last_item->next;
		last_item->next = NULL;
	}
	else {
		new_array->next = array->next;
		item_t *last_item = array->get(array, index+count-1);
		array->next = last_item->next;
		last_item->next = NULL;
	}
	array->length = array->length-count;
	return new_array;
}

int remove_items(da_t *array, int index, int count) {
	item_t *item = array->next;
	if (index+count > array->length) {
		printf("%s:%d: %d is out of range\n", __FILE__, __LINE__, index);
		return 1;
	}
	if (count == 0) {
		printf("%s:%d: Zero count splice\n", __FILE__, __LINE__);
		return 1;
	}
	da_t *new_array = malloc(sizeof(da_t));
	*new_array = DefaultArray;
	new_array->length = count;
	if (index != 0) {
		item_t *item = array->get(array, index-1);
		new_array->next = item->next;
		item_t *last_item = array->get(array, index+count-1);
		item->next = last_item->next;
		item = item->next;
		while (item != last_item->next) {
			free(item);
			item = item->next;
		}
	}
	else {
		item_t *last_item = array->get(array, index+count-1);
		array->next = last_item->next;
		item_t *item = array->next;
		while (item != last_item->next) {
			free(item);
			item = item->next;
		}
	}
	array->length = array->length-count;
	return 0;
}

int insertBefore_item(da_t *array, int index, int count, ...) {
	if (index > array->length) {
		printf("%s:%d: %d out of range\n", __FILE__, __LINE__, index);
		return 1;
	}
	if (count == 0) {
		printf("%s:%d: Zero count insert\n", __FILE__, __LINE__);
		return 1;
	}
	va_list ap;
	itemType i;
	
	va_start(ap, count);
	if (index == 0) {
		item_t *last_item = array->next;
		itemType value = va_arg(ap, itemType);
		array->next = Item;
		item_t *item = array->next;
		if (item == NULL) {
			printf("%s:%d: Couldn't add new item\n", __FILE__, __LINE__);
			return 1;
		}
		item->value = value;
		array->length++;
		for (int i = 1; i < count; i++) {
			itemType value = va_arg(ap, itemType);
			item->next = Item;
			item = item->next;
			if (item == NULL) {
				printf("%s:%d: Couldn't add new item\n", __FILE__, __LINE__);
				return 1;
			}
			item->value = value;
			array->length++;
		}
		item->next = last_item;
	}
	else {
		item_t *item = array->get(array, index-1);
		item_t *last_item = item->next;
		for (int i = 0; i < count; i++) {
			itemType value = va_arg(ap, itemType);
			item->next = Item;
			item = item->next;
			if (item == NULL) {
				printf("%s:%d: Couldn't add new item\n", __FILE__, __LINE__);
				return 1;
			}
			item->value = value;
			array->length++;
		}
		item->next = last_item;
	}
	va_end(ap);
	return 0;
}

int insertAfter_item(da_t *array, int index, int count, ...) {
	if (index > array->length) {
		printf("%s:%d: %d out of range\n", __FILE__, __LINE__, index);
		return 1;
	}
	if (count == 0) {
		printf("%s:%d: Zero count insert\n", __FILE__, __LINE__);
		return 1;
	}
	va_list ap;
	itemType i;
	
	va_start(ap, count);
	item_t *item = array->get(array, index);
	item_t *last_item = item->next;
	for (int i = 0; i < count; i++) {
		itemType value = va_arg(ap, itemType);
		item->next = Item;
		item = item->next;
		if (item == NULL) {
			printf("%s:%d: Couldn't add new item\n", __FILE__, __LINE__);
			return 1;
		}
		item->value = value;
		array->length++;
	}
	item->next = last_item;
	va_end(ap);
	return 0;
}

int main() {
	da_t *array = Array;
	*array = DefaultArray;
	array->append(array, 5, 1, 2, 3, 4, 5);
	array->insertBefore(array, 3, 2, 6, 7);
	array->insertAfter(array, 1, 2, 8, 9);
	array->append(array, 1, 10);
	array->remove(array, array->length-2, 2);
	item_t *item = array->next;
	while(item != NULL) {
		printf("%d\n", item->value);
		item = item->next;
	}
	return 0;
}
