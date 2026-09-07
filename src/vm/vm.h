#ifndef VM_H
#define VM_H

#include <stdbool.h>

#include "util/chunk/chunk.h"
#include "util/table/table.h"
#include "util/value/value.h"

#define EXECUTE_OK 0
#define EXECUTE_ERR 1

#define STACK_MAX 256

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *stackTop;

	Table strings;
	Table globals;
	Object *objects;
} VirtualMachine;

void initVirtualMachine(VirtualMachine *vm, Chunk *chunk);
void freeVirtualMachine(VirtualMachine *vm);

int execute(VirtualMachine *vm, bool debug);

#endif
