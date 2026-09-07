#include "scanner.h"
#include "scanner_helpers.h"

void initScanner(Scanner *s, const char *src) {
	s->start = src;
	s->cur = src;
	s->line = 1;
}

Token scanToken(Scanner *s) {
	skipWhitespace(s);
	s->start = s->cur;

	if (isAtEnd(s)) {
		return makeToken(s, TKN_EOF);
	}

	char c = advanceToken(s);

	switch (c) {
		case '+': return makeToken(s, TKN_PLUS);
		case '-': return makeToken(s, TKN_MINUS);
		case '*': return makeToken(s, TKN_STAR);
		case '^': return makeToken(s, TKN_CARET);
		case '%': return makeToken(s, TKN_PERCENT);
		case '(': return makeToken(s, TKN_LEFT_PAREN);
		case ')': return makeToken(s, TKN_RIGHT_PAREN);
		case '{': return makeToken(s, TKN_LEFT_BRACE);
		case '}': return makeToken(s, TKN_RIGHT_BRACE);
		case '.': return makeToken(s, TKN_DOT);
		case ',': return makeToken(s, TKN_COMMA);
		case ';': return makeToken(s, TKN_SEMICOLON);
		case '&': return makeToken(s, TKN_AND);
		case '|': return makeToken(s, TKN_OR);
		case '!': return makeToken(s, matchToken(s, '=') ? TKN_BANG_EQUAL : TKN_BANG);
		case '=': return makeToken(s, matchToken(s, '=') ? TKN_EQUAL_EQUAL : TKN_EQUAL);
		case '>': return makeToken(s, matchToken(s, '=') ? TKN_GREATER_EQUAL : TKN_GREATER);
		case '<': return makeToken(s, matchToken(s, '=') ? TKN_LESS_EQUAL : TKN_LESS);
		case '/': {
			if (peekToken(s, 1) == '/') {
				while (peekToken(s, 0) != '\n') {
					s->cur++;
				}
			} else {
				return makeToken(s, TKN_SLASH);
			}
			break;
		}
		case '"': {
			return readString(s);
		}
		default: {
			if (isDigit(c)) {
				return readNumber(s);
			} else if (isAlpha(c)) {
				return readIdentifier(s);
			}
		}
	}

	return errorToken(s, "unexpected character encountered");
}
