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

static bool readerDone() {
    return *reader.cursor == '\0';
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

static char __next() {
    return *(reader.cursor++);
}

Token readToken() {
    reader.start = reader.cursor;
    if (readerDone()) return makeToken(TOKEN_EOF);

    char c = __next();

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
    }

    return makeError("unexpected character: %s", reader.start[0]);
}