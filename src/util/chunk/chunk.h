#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#include "util/value/value.h"

// clang-format off
typedef enum {
	OP_CONST, OP_CONST_LONG,
	OP_DEF_GLOBAL, OP_DEF_GLOBAL_LONG,
	OP_GET_GLOBAL, OP_GET_GLOBAL_LONG,
	OP_SET_GLOBAL, OP_SET_GLOBAL_LONG,
	OP_GET_LOCAL, OP_GET_LOCAL_LONG,
	OP_SET_LOCAL, OP_SET_LOCAL_LONG,

	OP_TRUE, OP_FALSE,
	OP_NULL, OP_NEGATE, OP_NOT,
	OP_ADD, OP_SUBTRACT,
	OP_MULTIPLY, OP_DIVIDE,
	OP_EXPONENT, OP_MODULO,
	OP_AND, OP_OR,
	OP_EQUAL, OP_GREATER, OP_LESS,

	OP_JUMP, OP_JUMP_IF_FALSE,
	OP_LOOP,

	OP_PRINT, OP_POP, OP_RETURN
} Opcode;
// clang-format on

typedef struct {
	uint8_t *code;
	int *lines;
	int len, cap;
	ValueList constants;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);

void writeIndex(Chunk *chunk, int idx, int line);

int addConstant(Chunk *chunk, Value value);

#endif
