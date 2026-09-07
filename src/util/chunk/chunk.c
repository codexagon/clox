#include "chunk.h"
#include "util/memory/memory.h"

void initChunk(Chunk *chunk) {
	chunk->len = 0;
	chunk->cap = 8;
	chunk->code = MEM_ALLOCATE(chunk->cap * sizeof(uint8_t));
	chunk->lines = MEM_ALLOCATE(chunk->cap * sizeof(int));
	initValueList(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
	if (chunk->len >= chunk->cap) {
		chunk->code = MEM_RESIZE(chunk->code, chunk->cap * sizeof(uint8_t), chunk->cap * 2 * sizeof(uint8_t));
		chunk->lines = MEM_RESIZE(chunk->lines, chunk->cap * sizeof(int), chunk->cap * 2 * sizeof(int));
		chunk->cap *= 2;
	}

	chunk->code[chunk->len] = byte;
	chunk->lines[chunk->len] = line;
	chunk->len++;
}

void freeChunk(Chunk *chunk) {
	freeValueList(&chunk->constants);
	MEM_FREE(chunk->code);
	MEM_FREE(chunk->lines);
	chunk->len = 0;
	chunk->cap = 0;
	chunk->code = NULL;
}

void writeIndex(Chunk *chunk, int idx, int line) {
	if (idx >= UINT8_MAX + 1) {
		writeChunk(chunk, (idx & 0xff0000) >> 16, line);
		writeChunk(chunk, (idx & 0x00ff00) >> 8, line);
		writeChunk(chunk, (idx & 0x0000ff), line);
	} else {
		writeChunk(chunk, idx, line);
	}
}

int addConstant(Chunk *chunk, Value value) {
	writeValueList(&chunk->constants, value);
	return chunk->constants.len - 1;
}
