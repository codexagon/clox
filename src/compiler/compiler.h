#ifndef COMPILER_H
#define COMPILER_H

#include <stdbool.h>
#include <stdlib.h>

#include "scanner/scanner.h"
#include "util/chunk/chunk.h"
#include "vm/vm.h"

#define COMPILE_OK 0
#define COMPILE_ERR 1

typedef struct {
	Token cur;
	Token prev;
	bool hadError;
	bool panic;
} Parser;

typedef struct {
	Token name;
	int depth;
} Local;

typedef struct {
	Local *locals;
	int localCount, localCap;
	int scopeDepth;

	Chunk *compilingChunk;
} Compiler;

typedef struct {
	Scanner scanner;
	Compiler compiler;
	Parser parser;
	VirtualMachine vm;

	bool canAssign;
} Common;

void initCompiler(Compiler *compiler);
bool compile(Common *c, const char *src, Chunk *chunk, bool debug);

void declaration(Common *c);
void varDeclaration(Common *c);

void statement(Common *c);
void exprStatement(Common *c);
void printStatement(Common *c);
void blockStatement(Common *c);
void ifStatement(Common *c);
void whileStatement(Common *c);
void forStatement(Common *c);

void expression(Common *c);
void number(Common *c);
void string(Common *c);
void literal(Common *c);
void variable(Common *c);
void grouping(Common *c);
void unary(Common *c);
void binary(Common *c);

// parser stuff
typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_EXPONENT,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Common *);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence infixPrecedence;
} ParseRule;

ParseRule *getRule(TokenType type);
void parsePrecedence(Common *c, Precedence prec);

#endif
