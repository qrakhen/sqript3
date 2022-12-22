#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "digester.h"
#include "reader.h"

typedef struct {
    Token current;
    Token previous;
    bool failed;
    bool panic;
} Digester;

Digester digester;

Segment* targetSegment;

static Segment* currentContext() {
    return targetSegment;
}

static void errorAt(Token* token, const char* message) {
    if (digester.panic) return;
    fprintf(stderr, "[line %d] err", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    digester.failed = true;
}

static void errorCurrent(const char* message) {
    digester.panic = true;
    errorAt(&digester.current, message);
}

static void error(const char* message) {
    errorAt(&digester.previous, message);
}

static void next() {
    digester.previous = digester.current;
    for (;;) {
        //digester.current = scanToken();
        if (digester.current.type != TOKEN_ERROR) break;
        errorCurrent(digester.current.start);
    }
}

static void digestToken(TokenType type, const char* message) {
    if (digester.current.type == type) {
        next();
        return;
    }
    errorCurrent(message);
}

static void writeByte(byte value) {
    writeSegment(currentContext(), value, digester.previous.line);
}

static void writeOperation(byte code, byte value) {
    writeByte(code);
    writeByte(value);
}

static void writeReturn() {
    writeByte(OP_RETURN);
}

static void finish() {
    writeReturn();
}

static void digestExpression() {

}

bool digest(const char* source, Segment* segment) {
    initReader(source);
    targetSegment = segment;

    digester.failed = digester.panic = false;

    next();
    digestExpression();
    digestToken(TOKEN_EOF, "unexpected end of expression.");
    finish();
    return !digester.failed;
}
