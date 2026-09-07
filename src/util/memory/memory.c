#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "util/object/object.h"
#include "vm/vm.h"

void *allocateMem(void *ptr, size_t old, size_t new) {
	if (new == 0) {
		free(ptr);
		return NULL;
	} else if (old == 0) {
		void *p = calloc(1, new);
		if (p == NULL) {
			fprintf(stderr, "memory allocation failed\n");
			exit(1);
		}
		return p;
	}

	void *reptr = realloc(ptr, new);
	if (reptr == NULL) {
		fprintf(stderr, "memory reallocation failed\n");
		exit(1);
	}

	return reptr;
}

static void freeObject(Object *obj) {
	switch (obj->type) {
		case OBJ_STRING: {
			ObjString *str = (ObjString *)obj;
			MEM_FREE(str->content);
			MEM_FREE(obj);
			break;
		}
	}
}

void freeObjects(VirtualMachine *vm) {
	Object *obj = vm->objects;
	while (obj != NULL) {
		Object *next = obj->next;
		freeObject(obj);
		obj = next;
	}
}
