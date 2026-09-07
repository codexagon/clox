#include <string.h>

#include "table.h"
#include "util/memory/memory.h"
#include "util/object/object.h"
#include "util/value/value.h"

void initTable(Table *table) {
	table->len = 0;
	table->cap = 16;
	table->entries = MEM_ALLOCATE(table->cap * sizeof(Entry));
	for (int i = 0; i < table->cap; i++) {
		table->entries[i].key = NULL;
		table->entries[i].value = NULL_VAL;
	}
}

void freeTable(Table *table) {
	MEM_FREE(table->entries);
}

static Entry *findEntry(Entry *entries, int cap, ObjString *key) {
	uint32_t idx = key->hash % cap;
	Entry *tombstone = NULL;

	for (;;) {
		Entry *entry = &entries[idx];
		if (entry->key == NULL) {
			if (IS_NULL(entry->value)) {
				return tombstone != NULL ? tombstone : entry;
			} else {
				if (tombstone == NULL) {
					tombstone = entry;
				}
			}
		} else if (entry->key == key) {
			return entry;
		}

		idx = (idx + 1) % cap;
	}
}

static void adjustCapacity(Table *table, int cap) {
	Entry *entries = MEM_ALLOCATE(cap * sizeof(Entry));

	table->len = 0;
	for (int i = 0; i < table->cap; i++) {
		Entry *entry = &table->entries[i];
		if (entry->key == NULL) {
			continue;
		}

		Entry *dest = findEntry(entries, cap, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->len++;
	}

	MEM_FREE(table->entries);
	table->entries = entries;
	table->cap = cap;
}

bool tableSet(Table *table, ObjString *key, Value value) {
	if (table->len >= table->cap * TABLE_MAX_LOAD) {
		adjustCapacity(table, table->cap * 2);
	}

	Entry *toInsert = findEntry(table->entries, table->cap, key);
	bool isNewKey = toInsert->key == NULL;
	if (isNewKey && IS_NULL(toInsert->value)) {
		table->len++;
	}

	toInsert->key = key;
	toInsert->value = value;
	return isNewKey;
}

bool tableGet(Table *table, ObjString *key, Value *value) {
	if (table->len == 0) {
		return false;
	}

	Entry *toGet = findEntry(table->entries, table->cap, key);
	if (toGet->key == NULL) {
		return false;
	}

	*value = toGet->value;
	return true;
}

bool tableDelete(Table *table, ObjString *key) {
	if (table->len == 0) {
		return false;
	}

	Entry *toDelete = findEntry(table->entries, table->cap, key);
	if (toDelete->key == NULL) {
		return false;
	}

	toDelete->key = NULL;
	toDelete->value = BOOL_VAL(true);
	return true;
}

void tableCopy(Table *src, Table *dest) {
	for (int i = 0; i < src->cap; i++) {
		Entry *entry = &src->entries[i];
		if (entry->key != NULL) {
			tableSet(dest, entry->key, entry->value);
		}
	}
}

ObjString *tableFindString(Table *table, const char *content, int len, uint32_t hash) {
	if (table->len == 0) {
		return NULL;
	}

	uint32_t idx = hash % table->cap;

	for (;;) {
		Entry *entry = &table->entries[idx];
		if (entry->key == NULL) {
			if (IS_NULL(entry->value)) {
				return NULL;
			}
		} else if (entry->key->len == len && entry->key->hash == hash && memcmp(entry->key->content, content, len) == 0) {
			return entry->key;
		}

		idx = (idx + 1) % table->cap;
	}
}
