#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/debug/debug.h"
#include "util/object/object.h"
#include "util/value/value.h"
#include "vm_helpers.h"

int runtimeError(VirtualMachine *vm, const char *format, ...) {
	size_t instruction = vm->ip - vm->chunk->code - 1;
	int line = vm->chunk->lines[instruction];
	fprintf(stderr, "[line %i] Runtime error: ", line);
	vm->stackTop = vm->stack;

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");

	return EXECUTE_ERR;
}

void stackPush(VirtualMachine *vm, Value val) {
	if (vm->stackTop - vm->stack >= UINT8_MAX + 1) {
		runtimeError(vm, "stack overflow");
		exit(2);
	}
	*vm->stackTop = val;
	vm->stackTop++;
}

Value stackPop(VirtualMachine *vm) {
	vm->stackTop--;
	return *vm->stackTop;
}

Value stackPeek(VirtualMachine *vm, int distance) {
	return vm->stackTop[-1 - distance];
}

void printDebugOutput(VirtualMachine *vm) {
	printf("S ");
	for (Value *v = vm->stack; v < vm->stackTop; v++) {
		printf("[");
		printValue(*v);
		printf("]");
	}
	printf("\n");
	disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
}

bool isObjectTruthy(Value v) {
	switch (AS_OBJ(v)->type) {
		case OBJ_STRING: return strlen(AS_CSTRING(v)) > 0;
	}

	return true;
}

bool isTruthy(Value v) {
	switch (v.type) {
		case VAL_NULL: return false;
		case VAL_BOOL: return AS_BOOL(v);
		case VAL_NUM: return AS_NUM(v) > 0;
		case VAL_OBJ: return isObjectTruthy(v);
	}

	return true;
}

int binaryOp(VirtualMachine *vm, char op) {
	if (!IS_NUM(stackPeek(vm, 0)) || !IS_NUM(stackPeek(vm, 1))) {
		return runtimeError(vm, "operands must be numbers");
	}

	double r = AS_NUM(stackPop(vm));
	double l = AS_NUM(stackPop(vm));

	switch (op) {
		case '-': {
			stackPush(vm, NUM_VAL(l - r));
			break;
		}
		case '*': {
			stackPush(vm, NUM_VAL(l * r));
			break;
		}
		case '/': {
			if (r == 0) {
				return runtimeError(vm, "division by 0 is not possible");
			}
			stackPush(vm, NUM_VAL(l / r));
			break;
		}
		case '^': {
			if (l == 0 && r == 0) {
				return runtimeError(vm, "0^0 is not possible");
			}
			stackPush(vm, NUM_VAL(pow(l, r)));
			break;
		}
		case '%': {
			stackPush(vm, NUM_VAL((int)l % (int)r));
			break;
		}
		case '>': {
			stackPush(vm, BOOL_VAL(l > r));
			break;
		}
		case '<': {
			stackPush(vm, BOOL_VAL(l < r));
			break;
		}
	}

	return EXECUTE_OK;
}
