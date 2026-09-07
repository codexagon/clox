#ifndef COMPILER_HELPERS_H
#define COMPILER_HELPERS_H

#include <stdbool.h>

#include "compiler.h"
#include "scanner/scanner.h"
#include "util/chunk/chunk.h"

Chunk *currentChunk(Common *c);

void advance(Common *c);
void consume(Common *c, TokenType type, const char *msg);
bool check(Parser *p, TokenType type);
bool match(Common *c, TokenType type);

void errorAt(Parser *p, Token *token, const char *msg);
void error(Parser *p, const char *msg);
void errorAtCurrent(Parser *p, const char *msg);
void emitByte(Common *c, uint8_t byte);
void emitBytes(Common *c, uint8_t a, uint8_t b);
void emitIndex(Common *c, int idx);
void emitConstant(Common *c, int idx, Opcode codeLong, Opcode code);
int emitJump(Common *c, Opcode code);
void emitLoop(Common *c, int loopStart);

int identifierConstant(Common *c, Token *name);
void beginScope(Common *c);
void endScope(Common *c);
bool identifiersEqual(Token *a, Token *b);
void markInitialized(Compiler *c);
void addLocal(Compiler *c, Token name);
void declareLocalVar(Common *c);
int parseVariable(Common *c, const char *msg);
void defineVariable(Common *c, int global);
int resolveLocal(Common *c, Token *name);
void patchJump(Common *c, int offset);

#endif
