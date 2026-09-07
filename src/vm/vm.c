#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "util/chunk/chunk.h"
#include "util/memory/memory.h"
#include "util/object/object.h"
#include "util/value/value.h"
#include "vm.h"
#include "vm_helpers.h"

void initVirtualMachine(VirtualMachine *vm, Chunk *chunk) {
	vm->chunk = chunk;
	vm->ip = vm->chunk->code;
	vm->stackTop = vm->stack;

	initTable(&vm->strings);
	initTable(&vm->globals);
	vm->objects = NULL;
}

void freeVirtualMachine(VirtualMachine *vm) {
	vm->ip = NULL;
	freeObjects(vm);
	freeTable(&vm->strings);
	freeTable(&vm->globals);
}

int execute(VirtualMachine *vm, bool debug) {
	vm->ip = vm->chunk->code;

	for (;;) {
		if (debug) {
			printDebugOutput(vm);
		}
		switch (*vm->ip) {
			case OP_CONST: {
				int idx = vm->ip[1];
				Value constant = vm->chunk->constants.values[idx];
				stackPush(vm, constant);
				vm->ip++;
				break;
			}
			case OP_CONST_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				Value constant = vm->chunk->constants.values[idx];
				stackPush(vm, constant);
				vm->ip += 3;
				break;
			}
			case OP_DEF_GLOBAL: {
				int idx = vm->ip[1];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				if (!tableSet(&vm->globals, name, stackPeek(vm, 0))) {
					return runtimeError(vm, "trying to define an existing variable '%s'", name->content);
				}
				stackPop(vm);
				vm->ip++;
				break;
			}
			case OP_DEF_GLOBAL_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				if (!tableSet(&vm->globals, name, stackPeek(vm, 0))) {
					return runtimeError(vm, "trying to define an existing variable '%s'", name->content);
				}
				stackPop(vm);
				vm->ip += 3;
				break;
			}
			case OP_GET_GLOBAL: {
				int idx = vm->ip[1];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				Value val;
				if (!tableGet(&vm->globals, name, &val)) {
					return runtimeError(vm, "undefined variable '%s'", name->content);
				}
				stackPush(vm, val);
				vm->ip++;
				break;
			}
			case OP_GET_GLOBAL_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				Value val;
				if (!tableGet(&vm->globals, name, &val)) {
					return runtimeError(vm, "undefined variable '%s'", name->content);
				}
				stackPush(vm, val);
				vm->ip += 3;
				break;
			}
			case OP_SET_GLOBAL: {
				int idx = vm->ip[1];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				if (tableSet(&vm->globals, name, stackPeek(vm, 0))) {
					return runtimeError(vm, "undefined variable '%s'", name->content);
				}
				vm->ip++;
				break;
			}
			case OP_SET_GLOBAL_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				ObjString *name = AS_STRING(vm->chunk->constants.values[idx]);
				if (tableSet(&vm->globals, name, stackPeek(vm, 0))) {
					return runtimeError(vm, "undefined variable '%s'", name->content);
				}
				vm->ip += 3;
				break;
			}
			case OP_GET_LOCAL: {
				int idx = vm->ip[1];
				stackPush(vm, vm->stack[idx]);
				vm->ip++;
				break;
			}
			case OP_GET_LOCAL_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				stackPush(vm, vm->stack[idx]);
				vm->ip += 3;
				break;
			}
			case OP_SET_LOCAL: {
				int idx = vm->ip[1];
				vm->stack[idx] = stackPeek(vm, 0);
				vm->ip++;
				break;
			}
			case OP_SET_LOCAL_LONG: {
				int idx = (vm->ip[1] << 16) | (vm->ip[2] << 8) | vm->ip[3];
				vm->stack[idx] = stackPeek(vm, 0);
				vm->ip += 3;
				break;
			}
			case OP_TRUE: {
				stackPush(vm, BOOL_VAL(true));
				break;
			}
			case OP_FALSE: {
				stackPush(vm, BOOL_VAL(false));
				break;
			}
			case OP_NULL: {
				stackPush(vm, NULL_VAL);
				break;
			}
			case OP_NEGATE: {
				if (!IS_NUM(stackPeek(vm, 0))) {
					return runtimeError(vm, "operand to '-' must be a number");
				}
				vm->stackTop[-1] = NUM_VAL(-AS_NUM(vm->stackTop[-1]));
				break;
			}
			case OP_NOT: {
				vm->stackTop[-1] = BOOL_VAL(!isTruthy(vm->stackTop[-1]));
				break;
			}
			case OP_ADD: {
				if (isObjType(stackPeek(vm, 0), OBJ_STRING) && isObjType(stackPeek(vm, 1), OBJ_STRING)) {
					ObjString *r = AS_STRING(stackPop(vm));
					ObjString *l = AS_STRING(stackPop(vm));

					int length = r->len + l->len;
					char *concatenated = MEM_ALLOCATE((length + 1) * sizeof(char));
					memcpy(concatenated, l->content, l->len);
					memcpy(concatenated + l->len, r->content, r->len);
					concatenated[length] = '\0';

					ObjString *res = makeString(vm, concatenated, length);
					stackPush(vm, OBJ_VAL(res));
				} else if (IS_NUM(stackPeek(vm, 0)) && IS_NUM(stackPeek(vm, 0))) {
					double r = AS_NUM(stackPop(vm));
					double l = AS_NUM(stackPop(vm));
					stackPush(vm, NUM_VAL(l + r));
				} else {
					return runtimeError(vm, "operands to '+' must be both numbers or strings");
				}
				break;
			}
			case OP_SUBTRACT: {
				if (binaryOp(vm, '-') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_MULTIPLY: {
				if (binaryOp(vm, '*') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_DIVIDE: {
				if (binaryOp(vm, '/') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_EXPONENT: {
				if (binaryOp(vm, '^') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_MODULO: {
				if (binaryOp(vm, '%') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_AND: {
				Value b = stackPop(vm);
				Value a = stackPop(vm);
				stackPush(vm, BOOL_VAL(AS_BOOL(a) && AS_BOOL(b)));
				break;
			}
			case OP_OR: {
				Value b = stackPop(vm);
				Value a = stackPop(vm);
				stackPush(vm, BOOL_VAL(AS_BOOL(a) || AS_BOOL(b)));
				break;
			}
			case OP_EQUAL: {
				Value b = stackPop(vm);
				Value a = stackPop(vm);
				stackPush(vm, BOOL_VAL(valuesEqual(a, b)));
				break;
			}
			case OP_GREATER: {
				if (binaryOp(vm, '>') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_LESS: {
				if (binaryOp(vm, '<') == EXECUTE_ERR) {
					return EXECUTE_ERR;
				}
				break;
			}
			case OP_JUMP: {
				uint16_t offset = (uint16_t)((vm->ip[1] << 8) | vm->ip[2]);
				vm->ip += offset;
				vm->ip += 2;
				break;
			}
			case OP_JUMP_IF_FALSE: {
				uint16_t offset = (uint16_t)((vm->ip[1] << 8) | vm->ip[2]);
				if (!isTruthy(stackPeek(vm, 0))) {
					vm->ip += offset;
				}
				vm->ip += 2;
				break;
			}
			case OP_LOOP: {
				uint16_t offset = (uint16_t)((vm->ip[1] << 8) | vm->ip[2]);
				vm->ip -= offset;
				vm->ip += 2;
				break;
			}
			case OP_PRINT: {
				printValue(stackPop(vm));
				printf("\n");
				break;
			}
			case OP_POP: {
				stackPop(vm);
				break;
			}
			case OP_RETURN: {
				return EXECUTE_OK;
			}
			default: {
				return EXECUTE_ERR;
			}
		}

		vm->ip++;
	}

	return EXECUTE_OK;
}
