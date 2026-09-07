#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "compiler_helpers.h"
#include "scanner/scanner.h"
#include "util/debug/debug.h"
#include "util/memory/memory.h"
#include "util/object/object.h"

// clang-format off
ParseRule parseRules[] = {
    [TKN_NUMBER]        = {number,   NULL,   PREC_NONE      },
    [TKN_STRING]        = {string,   NULL,   PREC_NONE      },
    [TKN_IDENTIFIER]    = {variable, NULL,   PREC_NONE      },

    [TKN_PLUS]          = {NULL,     binary, PREC_TERM      },
    [TKN_MINUS]         = {unary,    binary, PREC_TERM      },
    [TKN_STAR]          = {NULL,     binary, PREC_FACTOR    },
    [TKN_SLASH]         = {NULL,     binary, PREC_FACTOR    },
    [TKN_CARET]         = {NULL,     binary, PREC_EXPONENT  },
    [TKN_PERCENT]       = {NULL,     binary, PREC_FACTOR    },
    [TKN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE      },
    [TKN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE      },
    [TKN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE      },
    [TKN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE      },
    [TKN_DOT]           = {NULL,     NULL,   PREC_NONE      },
    [TKN_COMMA]         = {NULL,     NULL,   PREC_NONE      },
    [TKN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE      },
    [TKN_AND]           = {NULL,     binary, PREC_AND       },
    [TKN_OR]            = {NULL,     binary, PREC_OR        },

    [TKN_EQUAL]         = {NULL,     NULL,   PREC_NONE      },
    [TKN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY  },
    [TKN_BANG]          = {unary,    NULL,   PREC_NONE      },
    [TKN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY  },
    [TKN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
    [TKN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
    [TKN_LESS]          = {NULL,     binary, PREC_COMPARISON},
    [TKN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},

    [TKN_LET]           = {NULL,     NULL,   PREC_NONE      },
    [TKN_NULL]          = {literal,  NULL,   PREC_NONE      },
    [TKN_TRUE]          = {literal,  NULL,   PREC_NONE      },
    [TKN_FALSE]         = {literal,  NULL,   PREC_NONE      },
    [TKN_IF]            = {NULL,     NULL,   PREC_NONE      },
    [TKN_ELSE]          = {NULL,     NULL,   PREC_NONE      },
    [TKN_WHILE]         = {NULL,     NULL,   PREC_NONE      },
    [TKN_FOR]           = {NULL,     NULL,   PREC_NONE      },
    [TKN_BREAK]         = {NULL,     NULL,   PREC_NONE      },
    [TKN_CONTINUE]      = {NULL,     NULL,   PREC_NONE      },
    [TKN_FUNCTION]      = {NULL,     NULL,   PREC_NONE      },
    [TKN_RETURN]        = {NULL,     NULL,   PREC_NONE      },
    [TKN_CLASS]         = {NULL,     NULL,   PREC_NONE      },

    [TKN_ERR]           = {NULL,     NULL,   PREC_NONE      },
    [TKN_EOF]           = {NULL,     NULL,   PREC_NONE      },
};
// clang-format on

void initCompiler(Compiler *compiler) {
	compiler->localCount = 0;
	compiler->localCap = 8;
	compiler->locals = MEM_ALLOCATE(compiler->localCap * sizeof(Local));
	compiler->scopeDepth = 0;
}

bool compile(Common *c, const char *src, Chunk *chunk, bool debug) {
	c->parser.hadError = false;
	c->parser.panic = false;

	initScanner(&c->scanner, src);
	initCompiler(&c->compiler);
	c->compiler.compilingChunk = chunk;

	advance(c);
	while (!match(c, TKN_EOF)) {
		declaration(c);
	}

	emitByte(c, OP_RETURN);
	if (debug && !c->parser.hadError) {
		disassembleChunk(currentChunk(c), "code");
		printf("\n");
	}

	MEM_FREE(c->compiler.locals);

	return !c->parser.hadError;
}

void declaration(Common *c) {
	if (match(c, TKN_LET)) {
		varDeclaration(c);
	} else {
		statement(c);
	}
}

// statement parsing fns
void varDeclaration(Common *c) {
	int idx = parseVariable(c, "expected variable name to be a valid identifier");

	if (match(c, TKN_EQUAL)) {
		expression(c);
	} else {
		emitByte(c, OP_NULL);
	}

	consume(c, TKN_SEMICOLON, "expected ';' after variable declaration");
	defineVariable(c, idx);
}

void statement(Common *c) {
	if (match(c, TKN_PRINT)) {
		printStatement(c);
	} else if (match(c, TKN_LEFT_BRACE)) {
		beginScope(c);
		blockStatement(c);
		endScope(c);
	} else if (match(c, TKN_IF)) {
		ifStatement(c);
	} else if (match(c, TKN_WHILE)) {
		whileStatement(c);
	} else if (match(c, TKN_FOR)) {
		forStatement(c);
	} else {
		exprStatement(c);
	}
}

void exprStatement(Common *c) {
	expression(c);
	consume(c, TKN_SEMICOLON, "expected ';' after expression");
	emitByte(c, OP_POP);
}

void printStatement(Common *c) {
	expression(c);
	consume(c, TKN_SEMICOLON, "expected ';' after expression");
	emitByte(c, OP_PRINT);
}

void blockStatement(Common *c) {
	while (!check(&c->parser, TKN_RIGHT_BRACE) && !check(&c->parser, TKN_EOF)) {
		declaration(c);
	}

	consume(c, TKN_RIGHT_BRACE, "expected '}' after block");
}

void ifStatement(Common *c) {
	consume(c, TKN_LEFT_PAREN, "expected '(' after 'if' keyword");
	expression(c);
	consume(c, TKN_RIGHT_PAREN, "expected ')' after condition");

	int thenJump = emitJump(c, OP_JUMP_IF_FALSE);
	emitByte(c, OP_POP);
	statement(c);

	int elseJump = emitJump(c, OP_JUMP);
	patchJump(c, thenJump);
	emitByte(c, OP_POP);

	if (match(c, TKN_ELSE)) {
		statement(c);
	}
	patchJump(c, elseJump);
}

void whileStatement(Common *c) {
	int loopStart = currentChunk(c)->len;
	consume(c, TKN_LEFT_PAREN, "expected '(' after 'while' keyword");
	expression(c);
	consume(c, TKN_RIGHT_PAREN, "expected ')' after condition");

	int exitJump = emitJump(c, OP_JUMP_IF_FALSE);
	emitByte(c, OP_POP);
	statement(c);
	emitLoop(c, loopStart);

	patchJump(c, exitJump);
	emitByte(c, OP_POP);
}

void forStatement(Common *c) {
	beginScope(c);
	consume(c, TKN_LEFT_PAREN, "expected '(' after 'while' keyword");
	if (match(c, TKN_SEMICOLON)) {
		// no initializer
	} else if (match(c, TKN_LET)) {
		varDeclaration(c);
	} else {
		exprStatement(c);
	}

	int loopStart = currentChunk(c)->len;
	int exitJump = -1;
	if (!match(c, TKN_SEMICOLON)) {
		expression(c);
		consume(c, TKN_SEMICOLON, "expected ';' after loop condition");

		exitJump = emitJump(c, OP_JUMP_IF_FALSE);
		emitByte(c, OP_POP);
	}

	if (!match(c, TKN_RIGHT_PAREN)) {
		int bodyJump = emitJump(c, OP_JUMP);
		int postStart = currentChunk(c)->len;
		expression(c);
		emitByte(c, OP_POP);
		consume(c, TKN_RIGHT_PAREN, "expected ')' after clauses");

		emitLoop(c, loopStart);
		loopStart = postStart;
		patchJump(c, bodyJump);
	}

	statement(c);
	emitLoop(c, loopStart);

	if (exitJump != -1) {
		patchJump(c, exitJump);
		emitByte(c, OP_POP);
	}

	endScope(c);
}

// expression parsing fns
void expression(Common *c) {
	parsePrecedence(c, PREC_ASSIGNMENT);
}

void number(Common *c) {
	double val = strtod(c->parser.prev.start, NULL);
	int idx = addConstant(currentChunk(c), NUM_VAL(val));
	emitConstant(c, idx, OP_CONST_LONG, OP_CONST);
}

void string(Common *c) {
	Value str = OBJ_VAL(copyString(&c->vm, c->parser.prev.start, c->parser.prev.len));
	int idx = addConstant(currentChunk(c), str);
	emitConstant(c, idx, OP_CONST_LONG, OP_CONST);
}

void literal(Common *c) {
	switch (c->parser.prev.type) {
		case TKN_TRUE: emitByte(c, OP_TRUE); break;
		case TKN_FALSE: emitByte(c, OP_FALSE); break;
		case TKN_NULL: emitByte(c, OP_NULL); break;
		default: return;
	}
}

void variable(Common *c) {
	Opcode getOp, getOpLong, setOp, setOpLong;
	int idx = resolveLocal(c, &c->parser.prev);
	if (idx != -1) {
		getOp = OP_GET_LOCAL;
		getOpLong = OP_GET_LOCAL_LONG;
		setOp = OP_SET_LOCAL;
		setOpLong = OP_SET_LOCAL_LONG;
	} else {
		Value str = OBJ_VAL(copyString(&c->vm, c->parser.prev.start, c->parser.prev.len));
		idx = AS_STRING(str)->constIdx;
		if (idx < 0) {
			error(&c->parser, "variable doesn't exist");
		}

		getOp = OP_GET_GLOBAL;
		getOpLong = OP_GET_GLOBAL_LONG;
		setOp = OP_SET_GLOBAL;
		setOpLong = OP_SET_GLOBAL_LONG;
	}

	if (c->canAssign && match(c, TKN_EQUAL)) {
		expression(c);
		emitConstant(c, idx, setOpLong, setOp);
	} else {
		emitConstant(c, idx, getOpLong, getOp);
	}
}

void grouping(Common *c) {
	expression(c);
	consume(c, TKN_RIGHT_PAREN, "expected ')' after expression");
}

void unary(Common *c) {
	TokenType opType = c->parser.prev.type;

	parsePrecedence(c, PREC_UNARY);

	switch (opType) {
		case TKN_MINUS: emitByte(c, OP_NEGATE); break;
		case TKN_BANG: emitByte(c, OP_NOT); break;
		default: return;
	}
}

void binary(Common *c) {
	TokenType opType = c->parser.prev.type;
	ParseRule *rule = getRule(opType);
	parsePrecedence(c, (Precedence)(rule->infixPrecedence + 1));

	switch (opType) {
		case TKN_PLUS: emitByte(c, OP_ADD); break;
		case TKN_MINUS: emitByte(c, OP_SUBTRACT); break;
		case TKN_STAR: emitByte(c, OP_MULTIPLY); break;
		case TKN_SLASH: emitByte(c, OP_DIVIDE); break;
		case TKN_CARET: emitByte(c, OP_EXPONENT); break;
		case TKN_PERCENT: emitByte(c, OP_MODULO); break;
		case TKN_AND: emitByte(c, OP_AND); break;
		case TKN_OR: emitByte(c, OP_OR); break;

		case TKN_EQUAL_EQUAL: emitByte(c, OP_EQUAL); break;
		case TKN_BANG_EQUAL: emitBytes(c, OP_EQUAL, OP_NOT); break;
		case TKN_GREATER: emitByte(c, OP_GREATER); break;
		case TKN_GREATER_EQUAL: emitBytes(c, OP_LESS, OP_NOT); break;
		case TKN_LESS: emitByte(c, OP_LESS); break;
		case TKN_LESS_EQUAL: emitBytes(c, OP_GREATER, OP_NOT); break;
		default: return;
	}
}

ParseRule *getRule(TokenType type) {
	return &parseRules[type];
}

void parsePrecedence(Common *c, Precedence prec) {
	advance(c);
	ParseFn prefixRule = getRule(c->parser.prev.type)->prefix;
	if (prefixRule == NULL) {
		error(&c->parser, "expected an expression");
		return;
	}

	c->canAssign = prec <= PREC_ASSIGNMENT;
	prefixRule(c);

	while (getRule(c->parser.cur.type)->infixPrecedence >= prec) {
		advance(c);
		ParseFn infixRule = getRule(c->parser.prev.type)->infix;
		infixRule(c);
	}

	if (c->canAssign && match(c, TKN_EQUAL)) {
		error(&c->parser, "invalid assignment target");
	}
}
