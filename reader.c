#include <stdio.h>
#include <string.h>

#include "common.h"
#include "reader.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Reader;

Reader reader;
void initScanner(const char* source) {
    reader.start = source;
    reader.current = source;
    reader.line = 1;
}

static bool isAlpha(char c) {
    return (
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_'
    );
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool done() {
    return *reader.current == '\0';
}

static char next() {
    return *(reader.current++);
}

static char current() {
    return *reader.current;
}

static char peek() {
    return done() ? '\0' : reader.current[1];
}

static bool match(char expected) {
    if (done()) return false;
    if (*reader.current != expected) return false;
    reader.current++;
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = reader.start;
    token.length = (int)(reader.current - reader.start);
    token.line = reader.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = reader.line;
    return token;
}

static void skipVoid() {
    for (;;) {
        char c = current();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                next();
                break;
            case '\n':
                reader.line++;
                next();
                break;
            case '#':
                while (current() != '\n' && !done()) next();
                break;
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (reader.current - reader.start == start + length &&
        memcmp(reader.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (reader.start[0]) {
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'q': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (reader.current - reader.start > 1) {
                switch (reader.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 2, "nq", TOKEN_FUNCTION);
                    case 'q': return TOKEN_FUNCTION;
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (reader.current - reader.start > 1) {
                switch (reader.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                    case 'y': return checkKeyword(2, 4, "peof", TOKEN_TYPEOF);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(current()) || isDigit(current())) next();
    return makeToken(identifierType());
}

static Token number() {
    while (isDigit(current())) next();
    if (current() == '.' && isDigit(peek())) {
        next();
        while (isDigit(current())) next();
    }
    return makeToken(TOKEN_NUMBER);
}

static Token string(char delimiter) {
    while (current() != delimiter && !done()) {
        if (current() == '\n') reader.line++;
        next();
    }
    if (done()) return errorToken("Unterminated string.");
    next();
    return makeToken(TOKEN_STRING);
}

Token readToken() {
    skipVoid();
    reader.start = reader.current;

    if (done()) return makeToken(TOKEN_EOF);

    char c = next();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TOKEN_GROUP_OPEN);
        case ')': return makeToken(TOKEN_GROUP_CLOSE);
        case '{': return makeToken(TOKEN_CONTEXT_OPEN);
        case '}': return makeToken(TOKEN_CONTEXT_CLOSE);
        case '[': return makeToken(TOKEN_ARRAY_OPEN);
        case ']': return makeToken(TOKEN_ARRAY_CLOSE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(match('~') ? TOKEN_THIS : TOKEN_DOT);
        case ':': return makeToken(match(':') ? TOKEN_PRINT : TOKEN_COLON);
        case '-': return match('-') ? makeToken(TOKEN_DECREMENT) : makeToken(TOKEN_MINUS);
        case '+': return match('+') ? makeToken(TOKEN_INCREMENT) : makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return match('~') ? makeToken(TOKEN_VAR) : makeToken(TOKEN_STAR);
        case '^':  
            return makeToken(
                match('~') ? TOKEN_SUPER : TOKEN_BITWISE_XOR);
        case '~': return match('?') ? makeToken(TOKEN_ELSE) : makeToken(TOKEN_BITWISE_NOT);
        case '?': return match('~') ? makeToken(TOKEN_IF) : makeToken(TOKEN_ERROR);
        case '&': return makeToken(match('&') ? TOKEN_AND : TOKEN_BITWISE_AND);
        case '|': return makeToken(match('|') ? TOKEN_OR : TOKEN_BITWISE_OR);
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : 
                match('&') ? TOKEN_REF : TOKEN_EQUAL);
        case '>': return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : (
                match('>') ? TOKEN_BITWISE_RIGHT : TOKEN_GREATER));
        case '<': return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : 
            match('<') ? TOKEN_BITWISE_LEFT : 
            match('~') ? TOKEN_EQUAL : 
            match(':') ? TOKEN_RETURN :
            match('+') ? TOKEN_ARRAY_ADD :
            match('-') ? TOKEN_ARRAY_REMOVE :
            match('&') ? TOKEN_REF : TOKEN_LESS);
        case '"': return string('"');
        case '\'': return string('\'');
    }

    return errorToken("Unexpected character.");
}
