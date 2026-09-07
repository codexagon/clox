#include <stdio.h>

#include "util/memory/memory.h"
#include "util/object/object.h"
#include "value.h"

void initValueList(ValueList *va) {
	va->len = 0;
	va->cap = 8;
	va->values = MEM_ALLOCATE(va->cap * sizeof(Value));
	for (int i = 0; i < va->cap; i++) {
		va->values[i] = NULL_VAL;
	}
}

void writeValueList(ValueList *va, Value v) {
	if (va->len >= va->cap) {
		va->values = MEM_RESIZE(va->values, va->cap * sizeof(Value), va->cap * 2 * sizeof(Value));
		va->cap *= 2;
	}

	va->values[va->len++] = v;
}

void freeValueList(ValueList *va) {
	MEM_FREE(va->values);
	va->len = 0;
	va->cap = 0;
	va->values = NULL;
}

bool valuesEqual(Value a, Value b) {
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
		case VAL_NULL: return true;
		case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
		case VAL_NUM: return AS_NUM(a) == AS_NUM(b);
		case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);
	}

	return false;
}

static void printObject(Value val) {
	switch (AS_OBJ(val)->type) {
		case OBJ_STRING: printf("%s", AS_CSTRING(val)); break;
	}
}

void printValue(Value v) {
	switch (v.type) {
		case VAL_NUM: printf("%.3f", AS_NUM(v)); break;
		case VAL_BOOL: printf(AS_BOOL(v) ? "true" : "false"); break;
		case VAL_NULL: printf("null"); break;
		case VAL_OBJ: printObject(v); break;
	}
}
