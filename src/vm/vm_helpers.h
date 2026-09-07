#ifndef VM_HELPERS_H
#define VM_HELPERS_H

#include "vm.h"

int runtimeError(VirtualMachine *vm, const char *format, ...);
void printDebugOutput(VirtualMachine *vm);

void stackPush(VirtualMachine *vm, Value val);
Value stackPop(VirtualMachine *vm);
Value stackPeek(VirtualMachine *vm, int distance);

bool isObjectTruthy(Value v);
bool isTruthy(Value v);
int binaryOp(VirtualMachine *vm, char op);

#endif
