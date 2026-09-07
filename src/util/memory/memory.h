#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "vm/vm.h"

#define MEM_ALLOCATE(size) allocateMem(NULL, 0, size)
#define MEM_RESIZE(ptr, oldSize, newSize) allocateMem(ptr, oldSize, newSize)
#define MEM_FREE(ptr) allocateMem(ptr, 0, 0)

void *allocateMem(void *ptr, size_t old, size_t new);

void freeObjects(VirtualMachine *vm);

#endif
