#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>

// clang-format off
typedef enum {
  TKN_NUMBER,
  TKN_IDENTIFIER,
  TKN_STRING,

  TKN_PLUS, TKN_MINUS, TKN_STAR,
  TKN_SLASH, TKN_CARET, TKN_PERCENT,
  TKN_LEFT_PAREN, TKN_RIGHT_PAREN,
  TKN_LEFT_BRACE, TKN_RIGHT_BRACE,
  TKN_DOT, TKN_COMMA, TKN_SEMICOLON,
  TKN_AND, TKN_OR,

  TKN_EQUAL, TKN_EQUAL_EQUAL,
  TKN_BANG, TKN_BANG_EQUAL,
  TKN_GREATER, TKN_GREATER_EQUAL,
  TKN_LESS, TKN_LESS_EQUAL,

  TKN_LET, TKN_NULL,
  TKN_TRUE, TKN_FALSE,
  TKN_IF, TKN_ELSE,
  TKN_WHILE, TKN_FOR,
	TKN_BREAK, TKN_CONTINUE,
  TKN_FUNCTION, TKN_RETURN,
  TKN_CLASS, TKN_PRINT,

	TKN_ERR, TKN_EOF
} TokenType;
// clang-format on

typedef struct {
	TokenType type;
	const char *start;
	int len, line;
} Token;

typedef struct {
	const char *start;
	const char *cur;
	int line;
} Scanner;

void initScanner(Scanner *s, const char *src);
Token scanToken(Scanner *s);

#endif
