#include <stdio.h>
#include <string.h>

#include "compiler_helpers.h"
#include "util/memory/memory.h"
#include "util/object/object.h"

Chunk *currentChunk(Common *c) {
	return c->compiler.compilingChunk;
}

void advance(Common *c) {
	c->parser.prev = c->parser.cur;

	for (;;) {
		c->parser.cur = scanToken(&c->scanner);
		if (c->parser.cur.type != TKN_ERR) {
			break;
		}
	}
}

void consume(Common *c, TokenType type, const char *msg) {
	if (c->parser.cur.type == type) {
		advance(c);
		return;
	}

	errorAtCurrent(&c->parser, msg);
}

bool check(Parser *p, TokenType type) {
	return p->cur.type == type;
}

bool match(Common *c, TokenType type) {
	if (c->parser.cur.type != type) {
		return false;
	}
	advance(c);
	return true;
}

void errorAt(Parser *p, Token *token, const char *msg) {
	if (p->panic) {
		return;
	}
	p->panic = true;

	fprintf(stderr, "[line %i] Error ", token->line);

	if (token->type == TKN_EOF) {
		fprintf(stderr, "at end");
	} else if (token->type == TKN_ERR) {

	} else {
		fprintf(stderr, "at '%.*s'", token->len, token->start);
	}

	fprintf(stderr, ": %s\n", msg);
	p->hadError = true;
}

void error(Parser *p, const char *msg) {
	errorAt(p, &p->prev, msg);
}

void errorAtCurrent(Parser *p, const char *msg) {
	errorAt(p, &p->cur, msg);
}

void emitByte(Common *c, uint8_t byte) {
	writeChunk(currentChunk(c), byte, c->parser.prev.line);
}

void emitBytes(Common *c, uint8_t a, uint8_t b) {
	emitByte(c, a);
	emitByte(c, b);
}

void emitIndex(Common *c, int idx) {
	writeIndex(currentChunk(c), idx, c->parser.prev.line);
}

void emitConstant(Common *c, int idx, Opcode codeLong, Opcode code) {
	(idx >= UINT8_MAX + 1) ? emitByte(c, codeLong) : emitByte(c, code);
	emitIndex(c, idx);
}

int emitJump(Common *c, Opcode code) {
	emitByte(c, code);
	emitBytes(c, 0xff, 0xff);
	return currentChunk(c)->len - 2;
}

void emitLoop(Common *c, int loopStart) {
	emitByte(c, OP_LOOP);
	int offset = currentChunk(c)->len - loopStart + 2;
	if (offset > UINT16_MAX) {
		error(&c->parser, "loop body too long");
	}
	emitByte(c, (offset >> 8) & 0xff);
	emitByte(c, offset & 0xff);
}

int identifierConstant(Common *c, Token *name) {
	Value str = OBJ_VAL(copyString(&c->vm, name->start, name->len));
	int idx = addConstant(currentChunk(c), str);
	AS_STRING(str)->constIdx = idx;
	return idx;
}

void beginScope(Common *c) {
	c->compiler.scopeDepth++;
}

void endScope(Common *c) {
	c->compiler.scopeDepth--;

	while (c->compiler.localCount > 0 && c->compiler.locals[c->compiler.localCount - 1].depth > c->compiler.scopeDepth) {
		emitByte(c, OP_POP);
		c->compiler.localCount--;
	}
}

bool identifiersEqual(Token *a, Token *b) {
	if (a->len != b->len) {
		return false;
	}

	return memcmp(a->start, b->start, a->len) == 0;
}

void markInitialized(Compiler *c) {
	c->locals[c->localCount - 1].depth = c->scopeDepth;
}

void addLocal(Compiler *c, Token name) {
	if (c->localCount >= c->localCap) {
		c->locals = MEM_RESIZE(c->locals, c->localCap, c->localCap * 2);
	}
	Local *local = &c->locals[c->localCount++];
	local->name = name;
	local->depth = -1;
}

void declareLocalVar(Common *c) {
	if (c->compiler.scopeDepth == 0) {
		return;
	}

	for (int i = c->compiler.localCount - 1; i >= 0; i--) {
		Local *local = &c->compiler.locals[i];
		if (local->depth != -1 && local->depth < c->compiler.scopeDepth) {
			break;
		}

		if (identifiersEqual(&c->parser.prev, &local->name)) {
			error(&c->parser, "variable already exists in current scope");
		}
	}

	addLocal(&c->compiler, c->parser.prev);
}

int parseVariable(Common *c, const char *msg) {
	consume(c, TKN_IDENTIFIER, msg);

	declareLocalVar(c);
	if (c->compiler.scopeDepth > 0) {
		return 0;
	}

	return identifierConstant(c, &c->parser.prev);
}

void defineVariable(Common *c, int global) {
	if (c->compiler.scopeDepth > 0) {
		markInitialized(&c->compiler);
		return;
	}
	emitConstant(c, global, OP_DEF_GLOBAL_LONG, OP_DEF_GLOBAL);
}

int resolveLocal(Common *c, Token *name) {
	for (int i = c->compiler.localCount - 1; i >= 0; i--) {
		Local local = c->compiler.locals[i];
		if (identifiersEqual(name, &local.name)) {
			if (local.depth == -1) {
				error(&c->parser, "cannot read local variable in its own initializer");
			}
			return i;
		}
	}

	return -1;
}

void patchJump(Common *c, int offset) {
	int jump = currentChunk(c)->len - offset - 2;
	if (jump > UINT16_MAX) {
		error(&c->parser, "too much code to jump over");
	}

	currentChunk(c)->code[offset] = (jump >> 8) & 0xff;
	currentChunk(c)->code[offset + 1] = jump & 0xff;
}
