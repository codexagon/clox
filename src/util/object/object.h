#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>
#include <stdint.h>

#include "util/value/value.h"
#include "vm/vm.h"

typedef enum { OBJ_STRING } ObjType;

#define AS_STRING(val) ((ObjString *)AS_OBJ(val))
#define AS_CSTRING(val) (((ObjString *)AS_OBJ(val))->content)

struct Object {
	ObjType type;
	struct Object *next;
};

struct ObjString {
	Object base;
	char *content;
	int len, constIdx;
	uint32_t hash;
};

bool isObjType(Value val, ObjType type);

Object *allocateObject(VirtualMachine *vm, ObjType type, size_t size);

ObjString *allocateString(VirtualMachine *vm, char *content, int len, uint32_t hash);
ObjString *makeString(VirtualMachine *vm, char *content, int len);
ObjString *copyString(VirtualMachine *vm, const char *content, int len);

#endif
