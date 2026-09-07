#ifndef SCANNER_HELPERS_H
#define SCANNER_HELPERS_H

#include <stdbool.h>

#include "scanner.h"

bool isAtEnd(Scanner *s);
bool isDigit(char c);
bool isAlpha(char c);

Token makeToken(Scanner *s, TokenType type);
Token errorToken(Scanner *s, const char *msg);

char peekToken(Scanner *s, int offset);
char advanceToken(Scanner *s);
bool matchToken(Scanner *s, char expected);

void skipWhitespace(Scanner *s);
TokenType keyword(Scanner *s, int start, int len, const char *remaining, TokenType type);
TokenType getIdentifierType(Scanner *s);
Token readIdentifier(Scanner *s);
Token readString(Scanner *s);
Token readNumber(Scanner *s);

#endif
