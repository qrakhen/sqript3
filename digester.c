#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "digester.h"
#include "digest.h"
#include "reader.h"

typedef struct {
    Token current;
    Token previous;
    bool failed;
    bool panic;
} Digester;

typedef enum {
    W_NONE,
    W_ASSIGNMENT,
    W_OR,
    W_AND,
    W_EQUALITY,
    W_COMPARISON,
    W_TERM,
    W_BITWISE,
    W_FACTOR,
    W_UNARY,
    W_CALL,
    W_PRIMARY
} Weight;

typedef void (*DigestCallback)();

typedef struct {
    DigestCallback prefix;
    DigestCallback infix;
    Weight weight;
} OpRule;

Digester digester;

Segment* __target;

static Segment* currentTarget() {
    return __target;
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
        digester.current = readToken();
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
    writeSegment(currentTarget(), value, digester.previous.line);
}

static void writeOperation(byte code, byte value) {
    writeByte(code);
    writeByte(value);
}

static void writeReturn() {
    writeByte(OP_RETURN);
}

static void digestExpression();
static OpRule* getRule(TokenType type);
static void digestWeight(Weight weight);

static byte makeConstant(Value value) {
    int c = registerConstant(currentTarget(), value);
    if (c > UINT8_MAX) {
        error("can not write beyond 255 constants in segment");
        return 0;
    }
    return (byte)c;
}

static void writeConstant(Value value) {
    writeOperation(OP_CONSTANT, makeConstant(value));
}

static void digestNumber() {
    double v = strtod(digester.previous.start, NULL);
    writeConstant(VAL_NUMBER(v));
}

static void finish() {
    writeReturn();
    if (digester.failed) {
        digestSegment(currentTarget(), "code");
    }
}

static void digestWeight(Weight weight) {
    next();
    DigestCallback pre = getRule(digester.previous.type)->prefix;
    if (pre == NULL) {
        error("expected something.");
        return;
    }
    
    pre();

    while (weight >= getRule(digester.current.type)->weight) {
        next();
        DigestCallback fn = getRule(digester.previous.type)->infix;
        if (fn == NULL)
            return;
        fn();
    }
}

static void digestExpression() {
    digestWeight(W_ASSIGNMENT);
}

static void digestGroup() {
    digestExpression();
    digestToken(TOKEN_GROUP_CLOSE, "expected closing ) after opening group with (");
}

static void digestUnary() {
    TokenType op = digester.previous.type;
    digestExpression();
    digestWeight(W_UNARY);
    switch (op) {
    case TOKEN_MINUS: writeByte(OP_NEG); break;
    default:
        return;
    }
}

static void digestBinary() {
    TokenType op = digester.previous.type;
    OpRule* rule = getRule(op);
    digestWeight((Weight)(rule->weight + 1));

    switch (op) {
    case TOKEN_PLUS: writeByte(OP_ADD); break;
    case TOKEN_MINUS: writeByte(OP_SUB); break;
    case TOKEN_STAR: writeByte(OP_MUL); break;
    case TOKEN_SLASH: writeByte(OP_DIV); break;
    case TOKEN_BITWISE_AND: writeByte(OP_BITWISE_AND); break;
    case TOKEN_BITWISE_OR: writeByte(OP_BITWISE_OR); break;
    case TOKEN_BITWISE_XOR: writeByte(OP_BITWISE_XOR); break;
    case TOKEN_BITWISE_NOT: writeByte(OP_BITWISE_NOT); break;
    default:
        return;
    }
}

static void digestLiteral() {
    switch (digester.previous.type) {
    case TOKEN_FALSE: writeByte(OP_FALSE); break;
    case TOKEN_NULL: writeByte(OP_NULL); break;
    case TOKEN_TRUE: writeByte(OP_TRUE); break;
    default:
        return;
    }
}

OpRule rules[] = {
    [TOKEN_GROUP_OPEN]          = { digestGroup,    NULL,           W_NONE },
    [TOKEN_GROUP_CLOSE]         = { NULL,           NULL,           W_NONE },
    [TOKEN_CONTEXT_OPEN]        = { NULL,           NULL,           W_NONE },
    [TOKEN_CONTEXT_CLOSE]       = { NULL,           NULL,           W_NONE },
    [TOKEN_COLLECTION_OPEN]     = { NULL,           NULL,           W_NONE },
    [TOKEN_COLLECTION_CLOSE]    = { NULL,           NULL,           W_NONE },
    [TOKEN_COMMA]               = { NULL,           NULL,           W_NONE },
    [TOKEN_DOT]                 = { NULL,           NULL,           W_NONE },
    [TOKEN_MINUS]               = { digestUnary,    digestBinary,   W_TERM},
    [TOKEN_PLUS]                = { NULL,           digestBinary,   W_TERM},
    [TOKEN_BITWISE_AND]         = { NULL,           digestBinary,   W_TERM},
    [TOKEN_BITWISE_OR]          = { NULL,           digestBinary,   W_TERM},
    [TOKEN_BITWISE_XOR]         = { NULL,           digestBinary,   W_TERM},
    [TOKEN_BITWISE_NOT]         = { digestUnary,    NULL,           W_UNARY},
    [TOKEN_SEMICOLON]           = { NULL,           NULL,           W_NONE },
    [TOKEN_SLASH]               = { NULL,           digestBinary,   W_FACTOR},
    [TOKEN_STAR]                = { NULL,           digestBinary,   W_FACTOR},
    [TOKEN_BANG]                = { NULL,           NULL,           W_NONE },
    [TOKEN_BANG_EQUAL]          = { NULL,           NULL,           W_NONE },
    [TOKEN_EQUAL]               = { NULL,           NULL,           W_NONE },
    [TOKEN_EQUAL_EQUAL]         = { NULL,           NULL,           W_NONE },
    [TOKEN_GREATER]             = { NULL,           NULL,           W_NONE },
    [TOKEN_GREATER_EQUAL]       = { NULL,           NULL,           W_NONE },
    [TOKEN_LESS]                = { NULL,           NULL,           W_NONE },
    [TOKEN_LESS_EQUAL]          = { NULL,           NULL,           W_NONE },
    [TOKEN_IDENTIFIER]          = { NULL,           NULL,           W_NONE },
    [TOKEN_STRING]              = { NULL,           NULL,           W_NONE },
    [TOKEN_NUMBER]              = { digestNumber,   NULL,           W_NONE },
    [TOKEN_AND]                 = { NULL,           NULL,           W_NONE },
    [TOKEN_CLASS]               = { NULL,           NULL,           W_NONE },
    [TOKEN_ELSE]                = { NULL,           NULL,           W_NONE },
    [TOKEN_FALSE]               = { digestLiteral,  NULL,           W_NONE },
    [TOKEN_FOR]                 = { NULL,           NULL,           W_NONE },
    [TOKEN_FUNCTION]            = { NULL,           NULL,           W_NONE },
    [TOKEN_IF]                  = { NULL,           NULL,           W_NONE },
    [TOKEN_NULL]                = { digestLiteral,  NULL,           W_NONE },
    [TOKEN_OR]                  = { NULL,           NULL,           W_NONE },
    [TOKEN_PRINT]               = { NULL,           NULL,           W_NONE },
    [TOKEN_RETURN]              = { NULL,           NULL,           W_NONE },
    [TOKEN_SUPER]               = { NULL,           NULL,           W_NONE },
    [TOKEN_THIS]                = { NULL,           NULL,           W_NONE },
    [TOKEN_TRUE]                = { digestLiteral,  NULL,           W_NONE },
    [TOKEN_VAR]                 = { NULL,           NULL,           W_NONE },
    [TOKEN_WHILE]               = { NULL,           NULL,           W_NONE },
    [TOKEN_ERROR]               = { NULL,           NULL,           W_NONE },
    [TOKEN_EOF]                 = { NULL,           NULL,           W_NONE },
};

static OpRule* getRule(TokenType type) {
    return &rules[type];
}

bool digest(const char* source, Segment* segment) {
    initReader(source);
    __target = segment;

    digester.failed = digester.panic = false;

    next();
    digestExpression();
    digestToken(TOKEN_EOF, "unexpected end of expression.");
    finish();
    return !digester.failed;
}
