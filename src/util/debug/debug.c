#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "util/chunk/chunk.h"
#include "util/value/value.h"

void disassembleChunk(Chunk *chunk, const char *name) {
	printf("----- %s -----\n", name);

	for (int offset = 0; offset < chunk->len;) {
		offset = disassembleInstruction(chunk, offset);
	}
}

static int simpleInstruction(const char *name, int offset) {
	printf("%s\n", name);
	return offset + 1;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
	if (strstr(name, "_LONG") != NULL) {
		int constIdx = (chunk->code[offset + 1] << 16) | (chunk->code[offset + 2] << 8) | chunk->code[offset + 3];
		printf("%-16s %6i '", name, constIdx);
		printValue(chunk->constants.values[constIdx]);
		printf("'\n");
		return offset + 4;
	} else {
		uint8_t constIdx = chunk->code[offset + 1];
		printf("%-16s %6i '", name, constIdx);
		printValue(chunk->constants.values[constIdx]);
		printf("'\n");
		return offset + 2;
	}
}

static int indexInstruction(const char *name, Chunk *chunk, int offset) {
	uint8_t idx = chunk->code[offset + 1];
	printf("%-16s %6i\n", name, idx);
	return offset + 2;
}

static int jumpInstruction(const char *name, int sign, Chunk *chunk, int offset) {
	uint16_t jump = (chunk->code[offset + 1] << 8) | (chunk->code[offset + 2]);
	printf("%-16s %6i -> %i\n", name, offset, offset + 3 + sign * jump);
	return offset + 3;
}

int disassembleInstruction(Chunk *chunk, int offset) {
	printf("%04i ", offset);
	if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
		printf("   | ");
	} else {
		printf("%4i ", chunk->lines[offset]);
	}

	uint8_t instruction = chunk->code[offset];

	switch (instruction) {
		case OP_CONST: return constantInstruction("OP_CONST", chunk, offset);
		case OP_CONST_LONG: return constantInstruction("OP_CONST_LONG", chunk, offset);
		case OP_DEF_GLOBAL: return constantInstruction("OP_DEF_GLOBAL", chunk, offset);
		case OP_DEF_GLOBAL_LONG: return constantInstruction("OP_DEF_GLOBAL_LONG", chunk, offset);
		case OP_GET_GLOBAL: return constantInstruction("OP_GET_GLOBAL", chunk, offset);
		case OP_GET_GLOBAL_LONG: return constantInstruction("OP_GET_GLOBAL_LONG", chunk, offset);
		case OP_SET_GLOBAL: return constantInstruction("OP_SET_GLOBAL", chunk, offset);
		case OP_SET_GLOBAL_LONG: return constantInstruction("OP_SET_GLOBAL_LONG", chunk, offset);
		case OP_GET_LOCAL: return indexInstruction("OP_GET_LOCAL", chunk, offset);
		case OP_GET_LOCAL_LONG: return indexInstruction("OP_GET_LOCAL_LONG", chunk, offset);
		case OP_SET_LOCAL: return indexInstruction("OP_SET_LOCAL", chunk, offset);
		case OP_SET_LOCAL_LONG: return indexInstruction("OP_SET_LOCAL_LONG", chunk, offset);

		case OP_TRUE: return simpleInstruction("OP_TRUE", offset);
		case OP_FALSE: return simpleInstruction("OP_FALSE", offset);
		case OP_NULL: return simpleInstruction("OP_NULL", offset);
		case OP_NEGATE: return simpleInstruction("OP_NEGATE", offset);
		case OP_NOT: return simpleInstruction("OP_NOT", offset);
		case OP_ADD: return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT: return simpleInstruction("OP_SUBTRACT", offset);
		case OP_MULTIPLY: return simpleInstruction("OP_MULTIPLY", offset);
		case OP_DIVIDE: return simpleInstruction("OP_DIVIDE", offset);
		case OP_EXPONENT: return simpleInstruction("OP_EXPONENT", offset);
		case OP_MODULO: return simpleInstruction("OP_MODULO", offset);
		case OP_AND: return simpleInstruction("OP_AND", offset);
		case OP_OR: return simpleInstruction("OP_OR", offset);
		case OP_EQUAL: return simpleInstruction("OP_EQUAL", offset);
		case OP_GREATER: return simpleInstruction("OP_GREATER", offset);
		case OP_LESS: return simpleInstruction("OP_LESS", offset);

		case OP_JUMP: return jumpInstruction("OP_JUMP", 1, chunk, offset);
		case OP_JUMP_IF_FALSE: return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
		case OP_LOOP: return jumpInstruction("OP_LOOP", -1, chunk, offset);

		case OP_PRINT: return simpleInstruction("OP_PRINT", offset);
		case OP_POP: return simpleInstruction("OP_POP", offset);
		case OP_RETURN: return simpleInstruction("OP_RETURN", offset);

		default: {
			printf("Unknown opcode '%i'\n", instruction);
			return offset + 1;
		}
	}
}
