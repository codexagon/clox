#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

#include "util/value/value.h"

#define TABLE_MAX_LOAD 0.75

typedef struct {
	ObjString *key;
	Value value;
} Entry;

typedef struct {
	Entry *entries;
	int len, cap;
} Table;

void initTable(Table *table);
void freeTable(Table *table);

bool tableSet(Table *table, ObjString *key, Value value);
bool tableGet(Table *table, ObjString *key, Value *value);
bool tableDelete(Table *table, ObjString *key);

void tableCopy(Table *src, Table *dest);
ObjString *tableFindString(Table *table, const char *content, int len, uint32_t hash);

#endif
