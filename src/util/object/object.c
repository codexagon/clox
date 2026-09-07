#include <string.h>

#include "object.h"
#include "util/memory/memory.h"
#include "util/table/table.h"

bool isObjType(Value val, ObjType type) {
	return IS_OBJ(val) && AS_OBJ(val)->type == type;
}

Object *allocateObject(VirtualMachine *vm, ObjType type, size_t size) {
	Object *obj = MEM_ALLOCATE(size);
	obj->type = type;

	obj->next = vm->objects;
	vm->objects = obj;

	return obj;
}

ObjString *allocateString(VirtualMachine *vm, char *content, int len, uint32_t hash) {
	ObjString *str = (ObjString *)allocateObject(vm, OBJ_STRING, sizeof(ObjString));

	str->content = content;
	str->len = len;
	str->hash = hash;
	str->constIdx = -1;
	tableSet(&vm->strings, str, NULL_VAL);
	return str;
}

static uint32_t hashString(const char *key, int len) {
	uint32_t hash = 2166136261u;

	for (int i = 0; i < len; i++) {
		hash ^= (uint32_t)key[i];
		hash *= 16777619;
	}

	return hash;
}

ObjString *makeString(VirtualMachine *vm, char *content, int len) {
	uint32_t hash = hashString(content, len);

	ObjString *interned = tableFindString(&vm->strings, content, len, hash);
	if (interned != NULL) {
		return interned;
	}

	return allocateString(vm, content, len, hash);
}

ObjString *copyString(VirtualMachine *vm, const char *content, int len) {
	uint32_t hash = hashString(content, len);

	ObjString *interned = tableFindString(&vm->strings, content, len, hash);
	if (interned != NULL) {
		return interned;
	}

	char *str = MEM_ALLOCATE((len + 1) * sizeof(char));
	memcpy(str, content, len);
	str[len] = '\0';
	return allocateString(vm, str, len, hash);
}
