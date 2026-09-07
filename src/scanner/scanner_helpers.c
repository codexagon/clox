#include <string.h>

#include "scanner_helpers.h"

bool isAtEnd(Scanner *s) {
	return *s->cur == '\0';
}

bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

Token makeToken(Scanner *s, TokenType type) {
	Token token;
	token.type = type;
	token.start = s->start;
	token.len = (int)(s->cur - s->start);
	token.line = s->line;
	return token;
}

Token errorToken(Scanner *s, const char *msg) {
	Token token;
	token.type = TKN_ERR;
	token.start = msg;
	token.len = (int)strlen(msg);
	token.line = s->line;
	return token;
}

char peekToken(Scanner *s, int offset) {
	char c = s->cur[offset];
	return c;
}

char advanceToken(Scanner *s) {
	s->cur++;
	return s->cur[-1];
}

bool matchToken(Scanner *s, char expected) {
	if (isAtEnd(s)) {
		return false;
	}
	if (*s->cur != expected) {
		return false;
	}
	s->cur++;
	return true;
}

void skipWhitespace(Scanner *s) {
	for (;;) {
		char c = peekToken(s, 0);
		switch (c) {
			case ' ':
			case '\r':
			case '\t': s->cur++; break;
			case '\n': {
				s->line++;
				s->cur++;
				break;
			}
			case '/': {
				if (peekToken(s, 1) == '/') {
					while (peekToken(s, 0) != '\n') {
						s->cur++;
					}
				}
			}
			default: return;
		}
	}
}

TokenType keyword(Scanner *s, int start, int len, const char *remaining, TokenType type) {
	if ((s->cur - s->start == start + len) && memcmp(s->start + start, remaining, len) == 0) {
		return type;
	}

	return TKN_IDENTIFIER;
}

TokenType getIdentifierType(Scanner *s) {
	switch (s->start[0]) {
		case 'b': return keyword(s, 1, 4, "reak", TKN_BREAK);
		case 'c': {
			if (s->cur - s->start >= 1) {
				switch (s->start[1]) {
					case 'l': return keyword(s, 2, 3, "ass", TKN_CLASS);
					case 'o': return keyword(s, 2, 6, "ntinue", TKN_CONTINUE);
				}
			}
			break;
		}
		case 'e': return keyword(s, 1, 3, "lse", TKN_ELSE);
		case 'f': {
			if (s->cur - s->start >= 1) {
				switch (s->start[1]) {
					case 'a': return keyword(s, 2, 3, "lse", TKN_FALSE);
					case 'o': return keyword(s, 2, 1, "r", TKN_FOR);
					case 'n': {
						if (!isAlpha(s->start[2]) && !isDigit(s->start[2])) {
							return TKN_FUNCTION;
						}
					}
				}
			}
			break;
		}
		case 'i': return keyword(s, 1, 1, "f", TKN_IF);
		case 'l': return keyword(s, 1, 2, "et", TKN_LET);
		case 'n': return keyword(s, 1, 3, "ull", TKN_NULL);
		case 'p': return keyword(s, 1, 4, "rint", TKN_PRINT);
		case 'r': return keyword(s, 1, 5, "eturn", TKN_RETURN);
		case 't': return keyword(s, 1, 3, "rue", TKN_TRUE);
		case 'w': return keyword(s, 1, 4, "hile", TKN_WHILE);
	}

	return TKN_IDENTIFIER;
}

Token readIdentifier(Scanner *s) {
	while (isAlpha(peekToken(s, 0)) || isDigit(peekToken(s, 0))) {
		advanceToken(s);
	}

	return makeToken(s, getIdentifierType(s));
}

Token readString(Scanner *s) {
	s->start = s->cur;
	while (peekToken(s, 0) != '"' && !isAtEnd(s)) {
		if (peekToken(s, 0) == '\n') {
			s->line++;
		}
		advanceToken(s);
	}

	if (isAtEnd(s)) {
		return errorToken(s, "unterminated string");
	}

	Token t = makeToken(s, TKN_STRING);
	advanceToken(s);

	return t;
}

Token readNumber(Scanner *s) {
	while (isDigit(peekToken(s, 0))) {
		advanceToken(s);
	}

	if (peekToken(s, 0) == '.' && isDigit(peekToken(s, 1))) {
		advanceToken(s);
		while (isDigit(peekToken(s, 0))) {
			advanceToken(s);
		}
	}

	return makeToken(s, TKN_NUMBER);
}
