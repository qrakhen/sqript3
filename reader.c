#include <stdio.h>

#include "common.h"
#include "reader.h"

typedef struct {
    const char* start;
    const char* cursor;
    int line;
} Reader;

Reader reader;

void initReader(const char* source) {
    reader.start = source;
    reader.cursor = source;
    reader.line = 0;
}

static char current() { return *reader.cursor; }
static char next() { return *(reader.cursor++); }
static bool done() { return *reader.cursor == '\0'; }
static char peek() { return done() ? '\0' : reader.cursor[1]; }

static bool match(char c) {
    if (done() || current() != c) return false;
    reader.cursor++;
    return true;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c == '_'));
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (
            (reader.cursor - reader.start == start + length) &&
            (memcmp(reader.start + start, rest, length) == 0)) 
        return type;
    return TOKEN_IDENTIFIER;
}

static TokenType getIdentifierType() {
    switch (reader.start[0]) {
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    case 't':
        if (reader.cursor - reader.start > 1) {
            switch (reader.start[1]) {
            case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'f':
        if (reader.cursor - reader.start > 1) {
            switch (reader.start[1]) {
            case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u': return checkKeyword(2, 2, "nq", TOKEN_FUNCTION);
            }
        }
        break;
    }
    return TOKEN_IDENTIFIER;
}

static Token makeToken(TokenType type) {
    Token t;
    t.type = type;
    t.start = reader.start;
    t.length = (int)(reader.cursor - reader.start);
    t.line = reader.line;
    return t;
}

static Token makeError(const char* message) {
    Token t;
    t.type = TOKEN_ERROR;
    t.start = message;
    t.length = (int)(strlen(message));
    t.line = reader.line;
    return t;
}

static Token makeString(char delimiter) {
    while (peek() != delimiter && !done()) {
        if (peek() == '\n') reader.line++;
        next();
    }
    if (done()) return makeError("unterminated string");
    next();
    return makeToken(TOKEN_STRING);
}

static Token makeNumber() {
    while (isDigit(peek())) next();
    if (peek() == '.' && isDigit(peek())) {
        next();
        while (isDigit(peek())) next();
    }
    return makeToken(TOKEN_NUMBER);
}

static Token makeIdentifier() {
    while (isAlpha(peek()) || isDigit(peek())) next();
    return makeToken(getIdentifierType());
}

static void skipVoid() {
    do {
        char c = peek();
        switch (c) {
        case '\r':
        case ' ':
        case '\t':
            next();
            break;
        case '/':
            if (peek() == '/') {
                while (peek() != '\n' && !done()) next();
                reader.line++;
            } else return;
            break;
        default:
            return;
        }
    } while (true);
}

Token readToken() {
    skipVoid();
    reader.start = reader.cursor;
    if (done()) return makeToken(TOKEN_EOF);

    char c = next();

    if (isDigit(c)) return makeNumber();
    if (isAlpha(c)) return makeIdentifier();

    switch (c) {
        case '(': return makeToken(TOKEN_GROUP_OPEN);
        case ')': return makeToken(TOKEN_GROUP_CLOSE);
        case '{': return makeToken(TOKEN_CONTEXT_OPEN);
        case '}': return makeToken(TOKEN_CONTEXT_CLOSE);
        case '[': return makeToken(TOKEN_COLLECTION_OPEN);
        case ']': return makeToken(TOKEN_COLLECTION_CLOSE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '+': return makeToken(TOKEN_PLUS);
        case '-': return makeToken(TOKEN_MINUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '^': return makeToken(TOKEN_BITWISE_XOR);
        case '~': return makeToken(TOKEN_BITWISE_NOT);
        case '&': return makeToken(match('&') ? TOKEN_AND : TOKEN_BITWISE_AND);
        case '|': return makeToken(match('|') ? TOKEN_OR : TOKEN_BITWISE_OR);
        // case & ? & opand logicand
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '>': return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : (
                current() == '>' ? TOKEN_BITWISE_SHIFT_RIGHT : TOKEN_GREATER));
        case '<': return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : (
                current() == '<' ? TOKEN_BITWISE_SHIFT_LEFT : TOKEN_LESS));
        case '"': return makeString('"'); 
        case '\'': return makeString('\'');
    }

    return makeError("unexpected character", reader.start[0]);
}